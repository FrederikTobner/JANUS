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

#ifndef X86_64_INTERNAL_DRIVERS_VGA_CONSOLE_H
#define X86_64_INTERNAL_DRIVERS_VGA_CONSOLE_H

/// @file arch/internal/drivers/vga_console.h
/// @brief VGA text-mode console backend (x86_64 internal).
///
/// Declares vga_console_init(), used by arch_console_probe() when the
/// bootloader reports DISPLAY_MODE_VGA_TEXT.

#include <arch/drivers/console.h>
#include <contracts/display.h>

/// @brief Initialize the VGA text-mode console.
///
/// @param cfg  Display configuration (mode == DISPLAY_MODE_VGA_TEXT).
/// @return Pointer to the VGA console's console_ops_t.
console_ops_t const * vga_console_init(display_info_t const * cfg);

#endif /* X86_64_INTERNAL_DRIVERS_VGA_CONSOLE_H */
