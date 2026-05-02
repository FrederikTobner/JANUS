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

/// @file mmu.h
/// @brief Page table manipulation and MMIO mapping API.
///
/// Provides functions to initialize page table manipulation and map device MMIO regions.

#ifndef PAGE_TABLES_MMU_H
#define PAGE_TABLES_MMU_H

#include <janus/errno.h>
#include <janus/types.h>

/// Initializes the MMU subsystem with the given parameters.
///
/// @param hhdm_offset The offset for the Higher Half Direct Mapping (HHDM).
/// @param kernel_phys_base The physical base address of the kernel.
/// @param kernel_virt_base The virtual base address of the kernel.
/// @return JANUS_OK on success, or a negative error code on failure.
error_t mmu_init(u64 hhdm_offset, phys_addr_t kernel_phys_base, virt_addr_t kernel_virt_base);

/// Maps a physical MMIO region into the virtual address space.
///
/// @param phys_addr  The starting physical address of the MMIO region.
/// @param size       The size of the MMIO region in bytes.
/// @param out_virt   On success, set to the mapped virtual address.
/// @return JANUS_OK on success, JANUS_ENOSPC if the VA window is full,
///         JANUS_ENOMEM if a page table allocation fails, or
///         JANUS_EFAULT if the MMU is not yet initialized.
error_t mmu_map_mmio(phys_addr_t phys_addr, u64 size, virt_addr_t * out_virt);

#endif /* PAGE_TABLES_MMU_H */
