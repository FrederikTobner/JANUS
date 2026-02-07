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

#ifndef PAGE_TABLES_MMU_H
#define PAGE_TABLES_MMU_H

#include <janus/types.h>

/**
 * @file mmu.h
 * @brief Page table manipulation and MMIO mapping API.
 *
 * Provides functions to initialize page table manipulation and map device MMIO regions.
 */

void mmu_init(u64 hhdm_offset, u64 kernel_phys_base, u64 kernel_virt_base);
u64 mmu_map_mmio(u64 phys_addr, u64 size);

#endif /* PAGE_TABLES_MMU_H */
