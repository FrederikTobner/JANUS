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

#ifndef ARCH_DRIVERS_CONSOLE_H
#define ARCH_DRIVERS_CONSOLE_H

/// @file arch/drivers/console.h
/// @brief Architecture console contract (Tier 2).
///
/// Declares the console_ops_t vtable that each concrete console device
/// implements, and arch_console_probe() which selects and initializes
/// the right device for the current display configuration.
///
/// Depends only on contracts/display.h and janus/types.h — does NOT
/// include <drivers/console.h>, avoiding the Tier-2 → Tier-1 reverse
/// include that existed in the old arch/drivers/tty.h.

#include <contracts/display.h>
#include <janus/types.h>

/// @brief Operations provided by a concrete text-console device.
///
/// Coordinates are character cells; colors are 0–15 palette indices
/// compatible with the ANSI/VGA 16-color scheme.
typedef struct {
    /// Write character @p c at cell (@p x, @p y) with given colors.
    void (*put_cell)(u16 x, u16 y, char c, u8 fg, u8 bg);
    /// Scroll the display up one row; fill the new bottom row with @p bg.
    void (*scroll)(u8 fg, u8 bg);
    /// Clear the entire display to @p bg.
    void (*clear)(u8 fg, u8 bg);
    /// Move the hardware cursor (no-op on devices without one).
    void (*set_cursor)(u16 x, u16 y);
    /// Return the display size in character cells.
    void (*get_size)(u16 * width, u16 * height);
} console_ops_t;

/// @brief Probe and initialize a console for the given display configuration.
///
/// Returns a pointer to the ops table for the initialized device, or NULL
/// if @p cfg->mode is not supported on this architecture.
///
/// @param cfg  Display configuration from the boot context.
/// @return Pointer to initialized console_ops_t, or NULL on failure.
console_ops_t const * arch_console_probe(display_info_t const * cfg);

#endif /* ARCH_DRIVERS_CONSOLE_H */
