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

/// @file kmalloc_test.h
/// @brief Kernel heap allocator self-test (build-flag guarded).
///
/// Only declared/compiled when JANUS_TEST_KMALLOC is defined. Used to verify
/// kmalloc/kcalloc/krealloc/kfree behave correctly before relying on them
/// elsewhere in the kernel.

#ifndef KMAIN_KMALLOC_TEST_H
#define KMAIN_KMALLOC_TEST_H

#ifdef JANUS_TEST_KMALLOC

/// @brief Run the kernel heap allocator self-test.
///
/// Exercises kmalloc(), kcalloc(), krealloc() and kfree() with a fixed
/// sequence of allocation patterns and validates the reported heap
/// statistics. Panics with a message identifying the failing case if any
/// check fails; otherwise returns normally so the kernel continues booting.
///
/// Must be called after mm_slab_alloc_init() has completed successfully.
void kmain_kmalloc_test(void);

#endif /* JANUS_TEST_KMALLOC */

#endif /* KMAIN_KMALLOC_TEST_H */
