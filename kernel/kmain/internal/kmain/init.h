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

#include <boot/context.h>
#include <janus/types.h>

/**
 * @brief Initialize the serial port for debugging output.
 *
 * @param ctx Boot context (provides HHDM offset and kernel base addresses)
 * @return true if serial was initialized successfully
 */
bool kinit_serial(boot_context_t const * ctx);

/**
 * @brief Initialize the TTY subsystem.
 *
 * Automatically selects the appropriate backend:
 * - VGA text mode for identity-mapped boot (Multiboot2)
 * - Framebuffer rendering for higher-half boot (Limine)
 *
 * @param ctx Boot context (provides display info)
 * @param serial_available Whether serial is available for logging
 * @return true if TTY was initialized successfully
 */
bool kinit_tty(boot_context_t const * ctx, bool serial_available);

#endif /* KMAIN_INIT_H */
