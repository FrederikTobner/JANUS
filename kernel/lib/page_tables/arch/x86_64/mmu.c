/*****************************************************************************
 * Copyright (C) 2025 by Frederik Tobner                                     *
 *                                                                           *
 * This file is part of JANUS.                                               *
 *                                                                           *
 * Permission to use, copy, modify, and distribute this software and its     *
 * documentation under the terms of the GNU Affero General Public License is *
 * hereby granted.                                                           *
 * No representations are made about the suitability of this software for    *
 * any purpose.                                                              *
 * It is provided "as is" without express or implied warranty.               *
 * See the <https://www.gnu.org/licenses/agpl-3.0.en.html>                   *
 * GNU Affero General Public License                                         *
 * License for more details.                                                 *
 ****************************************************************************/

/// @file arch/x86_64/mmu.c
/// @brief x86_64 page table manipulation and MMIO mapping.
///
/// Implements mmu_init() and mmu_map_mmio() for the 4-level (PML4 → PDPT → PD
/// → PT) x86_64 page table format. Mirrors the AArch64 implementation: a
/// static pool of page table pages and a bump-pointer virtual address allocator
/// for MMIO regions.
///
/// Page table entry format (4 KB pages, x86_64 long mode):
///   Bit  0  — Present
///   Bit  1  — Read/Write
///   Bit  2  — User/Supervisor (0 = supervisor only)
///   Bit  3  — PWT (page-level write-through)
///   Bit  4  — PCD (page-level cache disable) — set for MMIO
///   Bits 11:0 — flags (must be zero in PA bits)
///   Bits 51:12 — physical address (4 KB aligned)
///   Bit 63  — NX (no-execute)

#include <asm/barriers.h>
#include <asm/regs.h>
#include <asm/tlb.h>
#include <janus/attributes.h>
#include <janus/types.h>
#include <page_tables/mmu.h>

// --- page table entry flags ---------------------------------------------------

#define PTE_PRESENT       (1UL << 0)
#define PTE_RW            (1UL << 1)
#define PTE_PWT           (1UL << 3)
#define PTE_PCD           (1UL << 4) ///< Cache-disable — required for MMIO
#define PTE_NX            (1UL << 63)
#define PTE_ADDR_MASK     0x000FFFFFFFFFF000UL

// --- page table geometry ------------------------------------------------------

#define PAGE_SIZE         4096UL
#define ENTRIES_PER_TABLE 512UL

#define PML4_SHIFT        39
#define PDPT_SHIFT        30
#define PD_SHIFT          21
#define PT_SHIFT          12

#define PML4_INDEX(va)    (((va) >> PML4_SHIFT) & 0x1FF)
#define PDPT_INDEX(va)    (((va) >> PDPT_SHIFT) & 0x1FF)
#define PD_INDEX(va)      (((va) >> PD_SHIFT) & 0x1FF)
#define PT_INDEX(va)      (((va) >> PT_SHIFT) & 0x1FF)

// --- MMIO virtual address window ----------------------------------------------
//
// 128 GiB window in the kernel-private half of the address space.
// Must not overlap with the HHDM or the kernel image.
// Limine places the HHDM at 0xFFFF800000000000; the kernel image typically
// lands near 0xFFFFFFFF80000000. This window sits well clear of both.

#define MMIO_VIRT_BASE    0xFFFF900000000000UL
#define MMIO_VIRT_END     0xFFFF980000000000UL

// --- pool configuration -------------------------------------------------------

#ifndef JANUS_PAGE_TABLE_POOL_SIZE
#define JANUS_PAGE_TABLE_POOL_SIZE 16
#endif
#define PAGE_TABLE_POOL_SIZE JANUS_PAGE_TABLE_POOL_SIZE

// --- module state -------------------------------------------------------------

static u64 page_table_pool[PAGE_TABLE_POOL_SIZE][ENTRIES_PER_TABLE] __aligned(PAGE_SIZE);
static u32 pool_next_index;
static virt_addr_t mmio_virt_next;
static u64 g_hhdm_offset;
static u64 g_kernel_phys_base;
static u64 g_kernel_virt_base;
static bool g_initialized;

// --- forward declarations -----------------------------------------------------

static phys_addr_t mmu_virtual_to_physical_address(virt_addr_t virt);
static virt_addr_t mmu_physical_to_virtual_address(phys_addr_t phys);
static phys_addr_t mmu_alloc_page_table_phys(void);
static u64 * mmu_get_or_create_entry(phys_addr_t table_phys, u32 index, bool is_table_level);

// --- public API ---------------------------------------------------------------

__cold void mmu_init(u64 hhdm_offset, phys_addr_t kernel_phys_base, virt_addr_t kernel_virt_base)
{
    g_hhdm_offset = hhdm_offset;
    g_kernel_phys_base = kernel_phys_base;
    g_kernel_virt_base = kernel_virt_base;
    g_initialized = true;
    pool_next_index = 0;
    mmio_virt_next = MMIO_VIRT_BASE;
}

virt_addr_t mmu_map_mmio(phys_addr_t phys_addr, u64 size)
{
    if (UNLIKELY(!g_initialized)) {
        return 0;
    }
    u64 aligned_size = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    if (UNLIKELY(mmio_virt_next + aligned_size > MMIO_VIRT_END)) {
        return 0;
    }
    virt_addr_t virt_addr = mmio_virt_next;

    // CR3 holds the physical address of the PML4; mask off flag bits in [11:0].
    phys_addr_t pml4_phys = asm_read_cr3() & PTE_ADDR_MASK;

    phys_addr_t page_start = phys_addr & ~(PAGE_SIZE - 1);
    phys_addr_t page_end = page_start + aligned_size;
    virt_addr_t va = virt_addr;

    for (phys_addr_t pa = page_start; pa < page_end; pa += PAGE_SIZE, va += PAGE_SIZE) {
        u64 * pml4_entry = mmu_get_or_create_entry(pml4_phys, PML4_INDEX(va), true);
        if (UNLIKELY(!pml4_entry)) {
            return 0;
        }
        phys_addr_t pdpt_phys = *pml4_entry & PTE_ADDR_MASK;

        u64 * pdpt_entry = mmu_get_or_create_entry(pdpt_phys, PDPT_INDEX(va), true);
        if (UNLIKELY(!pdpt_entry)) {
            return 0;
        }
        phys_addr_t pd_phys = *pdpt_entry & PTE_ADDR_MASK;

        u64 * pd_entry = mmu_get_or_create_entry(pd_phys, PD_INDEX(va), true);
        if (UNLIKELY(!pd_entry)) {
            return 0;
        }
        phys_addr_t pt_phys = *pd_entry & PTE_ADDR_MASK;

        u64 * pt = (u64 *) mmu_physical_to_virtual_address(pt_phys);
        // Device memory: Present | RW | PWT | PCD | NX — no caching.
        pt[PT_INDEX(va)] = (pa & PTE_ADDR_MASK) | PTE_PRESENT | PTE_RW | PTE_PWT | PTE_PCD | PTE_NX;

        asm_tlb_invlpg(va);
    }
    // All page-table entries installed successfully — commit the VA window.
    mmio_virt_next += aligned_size;
    // x86_64 INVLPG is self-serialising — no additional fence needed.
    // An MFENCE after all mappings ensures subsequent loads see the new PTEs.
    asm_mfence();
    return virt_addr;
}

// --- helpers ------------------------------------------------------------------

static phys_addr_t mmu_virtual_to_physical_address(virt_addr_t virt)
{
    return virt - g_kernel_virt_base + g_kernel_phys_base;
}

static virt_addr_t mmu_physical_to_virtual_address(phys_addr_t phys)
{
    return phys + g_hhdm_offset;
}

static phys_addr_t mmu_alloc_page_table_phys(void)
{
    if (UNLIKELY(pool_next_index >= PAGE_TABLE_POOL_SIZE)) {
        return 0;
    }
    u64 * table = page_table_pool[pool_next_index++];
    for (u32 i = 0; i < ENTRIES_PER_TABLE; i++) {
        table[i] = 0;
    }
    return mmu_virtual_to_physical_address((virt_addr_t) table);
}

/// Walk or create one level of the page table hierarchy.
///
/// If @p is_table_level is true and the entry is not yet present, a new
/// zeroed page table page is allocated from the pool and the entry is
/// initialised to point to it (Present | RW).
///
/// @param table_phys    Physical address of the current-level table.
/// @param index         Entry index within that table (0–511).
/// @param is_table_level  True for PML4/PDPT/PD entries; false for PT entries.
/// @return Pointer to the entry (via HHDM), or NULL if pool exhausted.
static u64 * mmu_get_or_create_entry(phys_addr_t table_phys, u32 index, bool is_table_level)
{
    u64 * table = (u64 *) mmu_physical_to_virtual_address(table_phys);
    u64 * entry = &table[index];
    if (!is_table_level || (*entry & PTE_PRESENT)) {
        return entry;
    }
    phys_addr_t new_table_phys = mmu_alloc_page_table_phys();
    if (UNLIKELY(new_table_phys == 0)) {
        return (u64 *) 0;
    }
    *entry = new_table_phys | PTE_PRESENT | PTE_RW;
    return entry;
}
