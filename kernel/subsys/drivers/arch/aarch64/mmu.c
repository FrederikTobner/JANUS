/**
 * @file mmu.c
 * @brief aarch64 MMU helper for mapping MMIO regions
 *
 * This module provides page table manipulation routines for mapping
 * device MMIO regions into the kernel's virtual address space.
 *
 * IMPORTANT: This module is self-contained and does not depend on
 * any other subsystem. It must be initialized with mmu_init() before
 * any mapping operations can be performed.
 */

#include <arch/impl/drivers/mmu.h>
#include <janus/types.h>

/* ========== INTERNAL STATE ========== */

/* Configuration (set by mmu_init) */
static u64 g_hhdm_offset;
static u64 g_kernel_phys_base;
static u64 g_kernel_virt_base;
static bool g_initialized;

/* ========== PAGE TABLE CONSTANTS ========== */

/* Page table entry flags (AArch64) */
#define PTE_VALID            (1UL << 0)
#define PTE_TABLE            (1UL << 1)  /* For L0-L2: points to next level table */
#define PTE_PAGE             (1UL << 1)  /* For L3: this is a page entry */
#define PTE_AF               (1UL << 10) /* Access Flag (must be set) */
#define PTE_SH_ISH           (3UL << 8)  /* Inner Shareable */
#define PTE_SH_OSH           (2UL << 8)  /* Outer Shareable (for device memory) */
#define PTE_UXN              (1UL << 54) /* Unprivileged Execute Never */
#define PTE_PXN              (1UL << 53) /* Privileged Execute Never */

/* Memory attribute indices (must match MAIR_EL1 configuration) */
#define PTE_ATTR_IDX(idx)    ((u64) (idx) << 2)
/* Index 0 = Normal memory (typically 0xFF)
 * Index 1 = Device-nGnRnE (typically 0x00) */

/* Address mask for page table entries */
#define PTE_ADDR_MASK        0x0000FFFFFFFFF000UL

/* Page and table sizes */
#define PAGE_SIZE            4096UL
#define PAGE_SHIFT           12
#define ENTRIES_PER_TABLE    512

/* VA bit positions for 4-level paging (48-bit VA) */
#define L0_SHIFT             39
#define L1_SHIFT             30
#define L2_SHIFT             21
#define L3_SHIFT             12

/* Index extraction macros */
#define L0_INDEX(va)         (((va) >> L0_SHIFT) & 0x1FF)
#define L1_INDEX(va)         (((va) >> L1_SHIFT) & 0x1FF)
#define L2_INDEX(va)         (((va) >> L2_SHIFT) & 0x1FF)
#define L3_INDEX(va)         (((va) >> L3_SHIFT) & 0x1FF)

/* ========== MMIO VIRTUAL ADDRESS SPACE ========== */

/*
 * MMIO Mapping Strategy:
 *
 * We define a dedicated region in the upper half for MMIO mappings.
 * This is separate from the HHDM (which only covers RAM).
 *
 * For simplicity, we use a 1:1 scheme within our MMIO region:
 *   Virtual = MMIO_VIRT_BASE + (Physical & 0xFFFFFFFF)
 *
 * This gives us 4GB of MMIO space, which is more than enough for
 * typical device regions.
 *
 * MMIO_VIRT_BASE is chosen to not conflict with:
 *   - HHDM (typically 0xFFFF800000000000)
 *   - Kernel image (typically 0xFFFFFFFF80000000)
 */
#define MMIO_VIRT_BASE       0xFFFFFF0000000000UL

/* ========== PAGE TABLE POOL ========== */

/*
 * Static pool of page tables for MMIO mappings.
 * We allocate from .bss to avoid needing a dynamic allocator.
 *
 * 8 tables should be enough for a few MMIO regions:
 *   - 1 for L0 modifications (unlikely, but reserved)
 *   - 1-2 for L1
 *   - 1-2 for L2
 *   - 2-3 for L3
 */
#define PAGE_TABLE_POOL_SIZE 8

static u64 page_table_pool[PAGE_TABLE_POOL_SIZE][ENTRIES_PER_TABLE] __attribute__((aligned(PAGE_SIZE)));

static u32 pool_next_index;

/* ========== HELPER FUNCTIONS ========== */

/**
 * Convert kernel virtual address to physical address.
 * Works for addresses in the kernel image (.text, .data, .bss).
 */
static u64 kernel_virt_to_phys(u64 virt)
{
    return virt - g_kernel_virt_base + g_kernel_phys_base;
}

/**
 * Convert physical address to virtual address via HHDM.
 * Works for any physical address that's mapped by HHDM.
 */
static u64 phys_to_virt(u64 phys)
{
    return phys + g_hhdm_offset;
}

/**
 * Allocate a new page table from the static pool.
 * Returns physical address of the new table.
 * Returns 0 on pool exhaustion.
 */
static u64 alloc_page_table_phys(void)
{
    if (pool_next_index >= PAGE_TABLE_POOL_SIZE) {
        return 0; /* Pool exhausted */
    }

    u64 * table = page_table_pool[pool_next_index++];

    /* Zero the table */
    for (u32 i = 0; i < ENTRIES_PER_TABLE; i++) {
        table[i] = 0;
    }

    /* Convert to physical address */
    return kernel_virt_to_phys((u64) table);
}

/**
 * Get or create a page table entry, allocating intermediate tables as needed.
 * Returns pointer to the PTE, or NULL on allocation failure.
 *
 * @param table_phys Physical address of the current level table
 * @param index Index within the table (0-511)
 * @param is_table_level TRUE if this level should contain table descriptors
 *                       (L0, L1, L2), FALSE for L3 page descriptors
 */
static u64 * get_or_create_pte(u64 table_phys, u32 index, bool is_table_level)
{
    /* Access table through HHDM */
    u64 * table = (u64 *) phys_to_virt(table_phys);
    u64 * pte = &table[index];

    /* If this is L3 level or entry already exists, just return it */
    if (!is_table_level || (*pte & PTE_VALID)) {
        return pte;
    }

    /* Need to allocate a new table for the next level */
    u64 new_table_phys = alloc_page_table_phys();
    if (new_table_phys == 0) {
        return (u64 *) 0; /* Allocation failed */
    }

    /* Create table descriptor */
    *pte = new_table_phys | PTE_VALID | PTE_TABLE;
    return pte;
}

/* ========== PUBLIC API ========== */

void mmu_init(u64 hhdm_offset, u64 kernel_phys_base, u64 kernel_virt_base)
{
    g_hhdm_offset = hhdm_offset;
    g_kernel_phys_base = kernel_phys_base;
    g_kernel_virt_base = kernel_virt_base;
    g_initialized = true;
    pool_next_index = 0;
}

u64 mmu_map_mmio(u64 phys_addr, u64 size)
{
    if (!g_initialized) {
        return 0; /* Not initialized */
    }

    /* Calculate virtual address in our MMIO region */
    u64 virt_addr = MMIO_VIRT_BASE + (phys_addr & 0xFFFFFFFF);

    /* Read current TTBR1_EL1 (kernel page table base) */
    u64 ttbr1;
    __asm__ volatile("mrs %0, ttbr1_el1" : "=r"(ttbr1));
    u64 l0_phys = ttbr1 & PTE_ADDR_MASK;

    /* Map each page */
    u64 page_start = phys_addr & ~(PAGE_SIZE - 1);
    u64 page_end = (phys_addr + size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    for (u64 pa = page_start; pa < page_end; pa += PAGE_SIZE) {
        u64 va = MMIO_VIRT_BASE + (pa & 0xFFFFFFFF);

        /* Walk/create L0 -> L1 */
        u64 * l0_pte = get_or_create_pte(l0_phys, L0_INDEX(va), true);
        if (!l0_pte) {
            return 0;
        }
        u64 l1_phys = *l0_pte & PTE_ADDR_MASK;

        /* Walk/create L1 -> L2 */
        u64 * l1_pte = get_or_create_pte(l1_phys, L1_INDEX(va), true);
        if (!l1_pte) {
            return 0;
        }
        u64 l2_phys = *l1_pte & PTE_ADDR_MASK;

        /* Walk/create L2 -> L3 */
        u64 * l2_pte = get_or_create_pte(l2_phys, L2_INDEX(va), true);
        if (!l2_pte) {
            return 0;
        }
        u64 l3_phys = *l2_pte & PTE_ADDR_MASK;

        /* Access L3 table and create page mapping */
        u64 * l3_table = (u64 *) phys_to_virt(l3_phys);
        u64 * l3_pte = &l3_table[L3_INDEX(va)];

        /* Create page descriptor with device memory attributes */
        *l3_pte = (pa & PTE_ADDR_MASK) | PTE_VALID | PTE_PAGE | PTE_AF | PTE_SH_OSH /* Outer Shareable for device */
                  | PTE_UXN                    /* Never execute from device memory */
                  | PTE_PXN | PTE_ATTR_IDX(1); /* Device-nGnRnE memory type */
    }

    /* TLB invalidation for the mapped region */
    for (u64 va = virt_addr; va < virt_addr + size; va += PAGE_SIZE) {
        __asm__ volatile("tlbi vale1is, %0" ::"r"(va >> 12));
    }
    __asm__ volatile("dsb sy");
    __asm__ volatile("isb");

    return virt_addr;
}
