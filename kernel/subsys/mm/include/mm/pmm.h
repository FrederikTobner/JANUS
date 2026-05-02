/*****************************************************************************
 * Copyright (C) 2025 by Frederik Tobner                                     *
 *                                                                           *
 * This file is part of JANUS.                                              *
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

#ifndef MM_PMM_H
#define MM_PMM_H

/// @file pmm.h
/// @brief Physical Memory Manager public API
///
/// Bitmap-based PMM covering 4 GiB of physical address space.
/// Each bit represents one 4 KiB page frame (1 = free, 0 = used).
/// The bitmap is statically allocated in .bss (128 KiB).
///
/// Usage:
///   mm_pmm_init(&boot_context);   // Once, during kernel init
///   phys_addr_t page = mm_pmm_alloc_page();
///   mm_pmm_free_page(page);

#include <boot/memmap.h>
#include <janus/errno.h>
#include <janus/types.h>

/// @brief PMM usage statistics
typedef struct {
    u64 total_pages; ///< Total tracked page frames
    u64 free_pages;  ///< Currently free page frames
    u64 used_pages;  ///< Currently used page frames
} mm_pmm_stats_t;

/// @brief Initialize the physical memory manager
///
/// Must be called once, after boot_init(), before any mm_pmm_alloc_page()
/// calls. Parses the boot memory map and marks usable regions free.
/// The kernel image and the first 1 MiB are always reserved.
///
/// @param memmap          Physical memory map from boot_init()
/// @param kernel_phys_base Physical start address of the kernel image
/// @param kernel_phys_end  Physical end address of the kernel image
/// @return JANUS_OK on success, JANUS_EINVAL if memmap is NULL,
///         JANUS_ENOMEM if no usable memory was found
error_t mm_pmm_init(boot_memmap_t const * memmap, phys_addr_t kernel_phys_base, phys_addr_t kernel_phys_end);

/// @brief Allocate one physical page frame (4 KiB)
///
/// Returns the physical address of a free page and marks it used.
/// Returns 0 if no free frames remain (out of memory). Physical address 0
/// is always reserved (low 1 MiB), so 0 is a safe failure sentinel.
/// The returned address is always >= 1 MiB (low memory is never allocated).
///
/// @return Physical address of the allocated page (page-aligned), or 0 on OOM
phys_addr_t mm_pmm_alloc_page(void);

/// @brief Free a previously allocated physical page frame
///
/// Marks the page frame at @p phys as free. Traps (hardware fault) if
/// @p phys is not page-aligned, out of range, or already free (double-free).
///
/// @param phys Physical address returned by mm_pmm_alloc_page()
void mm_pmm_free_page(phys_addr_t phys);

/// @brief Query PMM usage statistics
///
/// @param stats Output buffer for statistics (must not be NULL)
void mm_pmm_get_stats(mm_pmm_stats_t * stats);

#endif /* MM_PMM_H */
