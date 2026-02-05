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

#ifndef ARCH_IMPL_DRIVERS_MMU_H
#define ARCH_IMPL_DRIVERS_MMU_H

/**
 * @file mmu.h
 * @brief Minimal MMU helper for aarch64 MMIO mapping.
 *
 * Provides functions to map device MMIO regions into the kernel's
 * virtual address space.
 */

#include <janus/types.h>

/**
 * @brief Initialize the MMU mapping module.
 *
 * Must be called before any mmu_map_mmio() calls. Provides the module
 * with the necessary address translation parameters.
 *
 * @param hhdm_offset Higher-half direct map offset from Limine
 * @param kernel_phys_base Physical base address of the kernel image
 * @param kernel_virt_base Virtual base address of the kernel image
 */
void mmu_init(u64 hhdm_offset, u64 kernel_phys_base, u64 kernel_virt_base);

/**
 * @brief Map a physical MMIO address to virtual space.
 *
 * Maps a device MMIO region with device memory attributes (non-cacheable,
 * no execute). The mapping is placed at a fixed virtual address based on
 * the physical address.
 *
 * @param phys_addr Physical address of the MMIO region
 * @param size Size of the region to map
 * @return Virtual address of the mapping, or 0 on failure
 */
u64 mmu_map_mmio(u64 phys_addr, u64 size);

#endif /* ARCH_IMPL_DRIVERS_MMU_H */
