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

#include <asm/barriers.h>
#include <asm/regs.h>
#include <asm/tlb.h>
#include <janus/attributes.h>
#include <janus/errno.h>
#include <janus/types.h>
#include <page_tables/mmu.h>

#define PAGE_TABLE_ENTRY_VALID         (1UL << 0)
#define PAGE_TABLE_ENTRY_TABLE         (1UL << 1)
#define PAGE_TABLE_ENTRY_PAGE          (1UL << 1)
#define PAGE_TABLE_ENTRY_AF            (1UL << 10)
#define PAGE_TABLE_ENTRY_SH_OSH        (2UL << 8)
#define PAGE_TABLE_ENTRY_UXN           (1UL << 54)
#define PAGE_TABLE_ENTRY_PXN           (1UL << 53)
#define PAGE_TABLE_ENTRY_ATTR_IDX(idx) ((u64) (idx) << 2)
#define PAGE_TABLE_ENTRY_ADDR_MASK     0x0000FFFFFFFFF000UL
#define PAGE_SIZE                      4096UL
#define ENTRIES_PER_TABLE              512
#define L0_SHIFT                       39
#define L1_SHIFT                       30
#define L2_SHIFT                       21
#define L3_SHIFT                       12
#define L0_INDEX(va)                   (((va) >> L0_SHIFT) & 0x1FF)
#define L1_INDEX(va)                   (((va) >> L1_SHIFT) & 0x1FF)
#define L2_INDEX(va)                   (((va) >> L2_SHIFT) & 0x1FF)
#define L3_INDEX(va)                   (((va) >> L3_SHIFT) & 0x1FF)
#define MMIO_VIRT_BASE                 0xFFFFFF0000000000UL
#define MMIO_VIRT_END                  0xFFFFFF8000000000UL

#ifndef JANUS_PAGE_TABLE_POOL_SIZE
#define JANUS_PAGE_TABLE_POOL_SIZE 16
#endif
#define PAGE_TABLE_POOL_SIZE JANUS_PAGE_TABLE_POOL_SIZE

typedef struct {
    u64 hhdm_offset;
    u64 kernel_phys_base;
    u64 kernel_virt_base;
    u32 pool_next;
    virt_addr_t mmio_virt_next;
    bool initialized;
} mmu_state_t;

static u64 g_page_table_pool[PAGE_TABLE_POOL_SIZE][ENTRIES_PER_TABLE] __aligned(PAGE_SIZE);
static mmu_state_t g_mmu;

static phys_addr_t mmu_virtual_to_physical_address(virt_addr_t virt);
static virt_addr_t mmu_physical_to_virtual_address(phys_addr_t phys);
static phys_addr_t mmu_alloc_page_table_phys(void);
static phys_addr_t * mmu_get_or_create_page_table_entry(phys_addr_t table_phys, u32 index, bool is_table_level);

__cold error_t mmu_init(u64 hhdm_offset, phys_addr_t kernel_phys_base, virt_addr_t kernel_virt_base)
{
    g_mmu.hhdm_offset = hhdm_offset;
    g_mmu.kernel_phys_base = kernel_phys_base;
    g_mmu.kernel_virt_base = kernel_virt_base;
    g_mmu.initialized = true;
    g_mmu.pool_next = 0;
    g_mmu.mmio_virt_next = MMIO_VIRT_BASE;
    return JANUS_OK;
}

error_t mmu_map_mmio(phys_addr_t phys_addr, u64 size, virt_addr_t * out_virt)
{
    if (UNLIKELY(!g_mmu.initialized)) {
        return JANUS_EFAULT;
    }
    u64 aligned_size = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    if (UNLIKELY(g_mmu.mmio_virt_next + aligned_size > MMIO_VIRT_END)) {
        return JANUS_ENOSPC;
    }
    virt_addr_t virt_addr = g_mmu.mmio_virt_next;

    u64 ttbr1 = asm_read_ttbr1_el1();
    phys_addr_t l0_phys = ttbr1 & PAGE_TABLE_ENTRY_ADDR_MASK;
    phys_addr_t page_start = phys_addr & ~(PAGE_SIZE - 1);
    phys_addr_t page_end = page_start + aligned_size;
    virt_addr_t va = virt_addr;
    for (phys_addr_t pa = page_start; pa < page_end; pa += PAGE_SIZE, va += PAGE_SIZE) {
        phys_addr_t * l0_pte = mmu_get_or_create_page_table_entry(l0_phys, L0_INDEX(va), true);
        if (!l0_pte) {
            return JANUS_ENOMEM;
        }
        phys_addr_t l1_phys = *l0_pte & PAGE_TABLE_ENTRY_ADDR_MASK;
        phys_addr_t * l1_pte = mmu_get_or_create_page_table_entry(l1_phys, L1_INDEX(va), true);
        if (!l1_pte) {
            return JANUS_ENOMEM;
        }
        phys_addr_t l2_phys = *l1_pte & PAGE_TABLE_ENTRY_ADDR_MASK;
        phys_addr_t * l2_pte = mmu_get_or_create_page_table_entry(l2_phys, L2_INDEX(va), true);
        if (!l2_pte) {
            return JANUS_ENOMEM;
        }
        phys_addr_t l3_phys = *l2_pte & PAGE_TABLE_ENTRY_ADDR_MASK;
        u64 * l3_table = (u64 *) mmu_physical_to_virtual_address(l3_phys);
        l3_table[L3_INDEX(va)] = (pa & PAGE_TABLE_ENTRY_ADDR_MASK) | PAGE_TABLE_ENTRY_VALID | PAGE_TABLE_ENTRY_PAGE |
                                 PAGE_TABLE_ENTRY_AF | PAGE_TABLE_ENTRY_SH_OSH | PAGE_TABLE_ENTRY_UXN |
                                 PAGE_TABLE_ENTRY_PXN | PAGE_TABLE_ENTRY_ATTR_IDX(1);
    }
    // All page-table entries installed successfully — commit the VA window.
    g_mmu.mmio_virt_next += aligned_size;
    for (virt_addr_t v = virt_addr; v < virt_addr + aligned_size; v += PAGE_SIZE) {
        asm_tlb_invalidate_page(v);
    }
    asm_barrier_full();
    asm_barrier_load();
    *out_virt = virt_addr;
    return JANUS_OK;
}

static phys_addr_t mmu_virtual_to_physical_address(virt_addr_t virt)
{
    return virt - g_mmu.kernel_virt_base + g_mmu.kernel_phys_base;
}

static virt_addr_t mmu_physical_to_virtual_address(phys_addr_t phys)
{
    return phys + g_mmu.hhdm_offset;
}

static phys_addr_t mmu_alloc_page_table_phys(void)
{
    if (UNLIKELY(g_mmu.pool_next >= PAGE_TABLE_POOL_SIZE)) {
        return 0;
    }
    virt_addr_t * table = g_page_table_pool[g_mmu.pool_next++];
    for (u32 i = 0; i < ENTRIES_PER_TABLE; i++) {
        table[i] = 0;
    }
    return mmu_virtual_to_physical_address((virt_addr_t) table);
}

static phys_addr_t * mmu_get_or_create_page_table_entry(phys_addr_t table_phys, u32 index, bool is_table_level)
{
    virt_addr_t * table = (virt_addr_t *) mmu_physical_to_virtual_address(table_phys);
    phys_addr_t * page_table_entry = &table[index];
    if (!is_table_level || (*page_table_entry & PAGE_TABLE_ENTRY_VALID)) {
        return page_table_entry;
    }
    phys_addr_t new_table_phys = mmu_alloc_page_table_phys();
    if (UNLIKELY(new_table_phys == 0)) {
        return (phys_addr_t *) 0;
    }
    *page_table_entry = new_table_phys | PAGE_TABLE_ENTRY_VALID | PAGE_TABLE_ENTRY_TABLE;
    return page_table_entry;
}
