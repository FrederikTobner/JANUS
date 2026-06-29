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

/// @file arch/drivers/tty.h
/// @brief TTY architecture contract.
///
/// Declares arch_tty_* functions implemented in arch/<ARCH>/tty.c

#ifndef ARCH_DRIVERS_TTY_H
#define ARCH_DRIVERS_TTY_H

#include <drivers/tty.h>
#include <janus/types.h>

/// @brief Initialize display hardware.
/// @param config Display configuration (framebuffer or VGA).
/// @return 0 on success, negative error code on failure.
error_t arch_tty_init(display_info_t const * config);

/// @brief Get display dimensions.
/// @param width Pointer to store width (may be NULL).
/// @param height Pointer to store height (may be NULL).
void arch_tty_get_size(u16 * width, u16 * height);

/// @brief Write character and color at position.
/// @param x Column position.
/// @param y Row position.
/// @param c Character to write.
/// @param fg Foreground color (0-15).
/// @param bg Background color (0-15).
void arch_tty_write_cell(u16 x, u16 y, char c, u8 fg, u8 bg);

/// @brief Set hardware cursor position.
/// @param x Column position.
/// @param y Row position.
void arch_tty_set_cursor(u16 x, u16 y);

#endif /* ARCH_DRIVERS_TTY_H */
