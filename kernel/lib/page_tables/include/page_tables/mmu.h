#ifndef PAGE_TABLES_MMU_H
#define PAGE_TABLES_MMU_H

#include <janus/types.h>

/**
 * @file mmu.h
 * @brief Page table manipulation and MMIO mapping API.
 *
 * Provides functions to initialize page table manipulation and map device MMIO regions.
 */

#ifdef __cplusplus
extern "C" {
#endif

void mmu_init(u64 hhdm_offset, u64 kernel_phys_base, u64 kernel_virt_base);
u64 mmu_map_mmio(u64 phys_addr, u64 size);

#ifdef __cplusplus
}
#endif

#endif /* PAGE_TABLES_MMU_H */
