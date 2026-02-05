#include <janus/types.h>
#include <page_tables/mmu.h>
#include <stdbool.h>

#define PTE_VALID            (1UL << 0)
#define PTE_TABLE            (1UL << 1)
#define PTE_PAGE             (1UL << 1)
#define PTE_AF               (1UL << 10)
#define PTE_SH_OSH           (2UL << 8)
#define PTE_UXN              (1UL << 54)
#define PTE_PXN              (1UL << 53)
#define PTE_ATTR_IDX(idx)    ((u64) (idx) << 2)
#define PTE_ADDR_MASK        0x0000FFFFFFFFF000UL
#define PAGE_SIZE            4096UL
#define ENTRIES_PER_TABLE    512
#define L0_SHIFT             39
#define L1_SHIFT             30
#define L2_SHIFT             21
#define L3_SHIFT             12
#define L0_INDEX(va)         (((va) >> L0_SHIFT) & 0x1FF)
#define L1_INDEX(va)         (((va) >> L1_SHIFT) & 0x1FF)
#define L2_INDEX(va)         (((va) >> L2_SHIFT) & 0x1FF)
#define L3_INDEX(va)         (((va) >> L3_SHIFT) & 0x1FF)
#define MMIO_VIRT_BASE       0xFFFFFF0000000000UL
#define PAGE_TABLE_POOL_SIZE 8
static u64 page_table_pool[PAGE_TABLE_POOL_SIZE][ENTRIES_PER_TABLE] __attribute__((aligned(PAGE_SIZE)));
static u32 pool_next_index;
static u64 g_hhdm_offset;
static u64 g_kernel_phys_base;
static u64 g_kernel_virt_base;
static bool g_initialized;
static u64 kernel_virt_to_phys(u64 virt)
{
    return virt - g_kernel_virt_base + g_kernel_phys_base;
}
static u64 phys_to_virt(u64 phys)
{
    return phys + g_hhdm_offset;
}
static u64 alloc_page_table_phys(void)
{
    if (pool_next_index >= PAGE_TABLE_POOL_SIZE) {
        return 0;
    }
    u64 * table = page_table_pool[pool_next_index++];
    for (u32 i = 0; i < ENTRIES_PER_TABLE; i++) {
        table[i] = 0;
    }
    return kernel_virt_to_phys((u64) table);
}
static u64 * get_or_create_pte(u64 table_phys, u32 index, bool is_table_level)
{
    u64 * table = (u64 *) phys_to_virt(table_phys);
    u64 * pte = &table[index];
    if (!is_table_level || (*pte & PTE_VALID)) {
        return pte;
    }
    u64 new_table_phys = alloc_page_table_phys();
    if (new_table_phys == 0) {
        return (u64 *) 0;
    }
    *pte = new_table_phys | PTE_VALID | PTE_TABLE;
    return pte;
}
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
        return 0;
    }
    u64 virt_addr = MMIO_VIRT_BASE + (phys_addr & 0xFFFFFFFF);
    u64 ttbr1;
    __asm__ volatile("mrs %0, ttbr1_el1" : "=r"(ttbr1));
    u64 l0_phys = ttbr1 & PTE_ADDR_MASK;
    u64 page_start = phys_addr & ~(PAGE_SIZE - 1);
    u64 page_end = (phys_addr + size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    for (u64 pa = page_start; pa < page_end; pa += PAGE_SIZE) {
        u64 va = MMIO_VIRT_BASE + (pa & 0xFFFFFFFF);
        u64 * l0_pte = get_or_create_pte(l0_phys, L0_INDEX(va), true);
        if (!l0_pte) {
            return 0;
        }
        u64 l1_phys = *l0_pte & PTE_ADDR_MASK;
        u64 * l1_pte = get_or_create_pte(l1_phys, L1_INDEX(va), true);
        if (!l1_pte) {
            return 0;
        }
        u64 l2_phys = *l1_pte & PTE_ADDR_MASK;
        u64 * l2_pte = get_or_create_pte(l2_phys, L2_INDEX(va), true);
        if (!l2_pte) {
            return 0;
        }
        u64 l3_phys = *l2_pte & PTE_ADDR_MASK;
        u64 * l3_table = (u64 *) phys_to_virt(l3_phys);
        u64 * l3_pte = &l3_table[L3_INDEX(va)];
        *l3_pte =
            (pa & PTE_ADDR_MASK) | PTE_VALID | PTE_PAGE | PTE_AF | PTE_SH_OSH | PTE_UXN | PTE_PXN | PTE_ATTR_IDX(1);
    }
    for (u64 va = virt_addr; va < virt_addr + size; va += PAGE_SIZE) {
        __asm__ volatile("tlbi vale1is, %0" ::"r"(va >> 12));
    }
    __asm__ volatile("dsb sy");
    __asm__ volatile("isb");
    return virt_addr;
}