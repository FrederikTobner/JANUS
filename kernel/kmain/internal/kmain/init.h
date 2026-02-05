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

#ifndef KMAIN_INIT_H
#define KMAIN_INIT_H

/**
 * @file init.h
 * @brief Kernel initialization routines.
 *
 * Provides functions for initializing kernel subsystems during early boot.
 */

#include <janus/types.h>

/**
 * @brief Initialize the serial port for debugging output.
 *
 * @param hhdm_offset The HHDM offset for physical to virtual address translation.
 *                    Pass 0 for identity-mapped boot (Multiboot2).
 *                    On x86_64 this is ignored (port I/O doesn't need HHDM).
 *                    On AArch64 this is required for MMIO access.
 * @return true if serial was initialized successfully
 */
bool kinit_serial(u64 hhdm_offset);

/**
 * @brief Initialize the TTY subsystem.
 *
 * Automatically selects the appropriate backend:
 * - VGA text mode for identity-mapped boot (Multiboot2)
 * - Framebuffer rendering for higher-half boot (Limine)
 *
 * @param hhdm_offset The HHDM offset (0 for identity-mapped)
 * @param fb_info Framebuffer response from Limine (NULL for Multiboot2)
 * @param serial_available Whether serial is available for logging
 * @return true if TTY was initialized successfully
 */
bool kinit_tty(u64 hhdm_offset, void * fb_info, bool serial_available);

#endif /* KMAIN_INIT_H */
