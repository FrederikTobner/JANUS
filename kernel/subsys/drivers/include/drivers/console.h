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

#ifndef DRIVERS_CONSOLE_H
#define DRIVERS_CONSOLE_H

/// @file drivers/console.h
/// @brief Text console driver — public API.
///
/// Architecture contract (arch_console_probe, console_ops_t) is in
/// <arch/drivers/console.h>.  Terminal policy (cursor tracking, scroll
/// triggering, character classification) is in src/console.c.
///
/// Backends (selected at init time via arch_console_probe):
///   - x86_64: VGA text mode or framebuffer text rendering
///   - aarch64: framebuffer text rendering

#include <contracts/display.h>
#include <janus/types.h>

/// @brief Logical text-console color indices (16-color ANSI-compatible palette).
///
/// Values are architecture-agnostic sequential indices 0–15.
/// x86_64 VGA text mode maps these directly to hardware nibble values.
/// Framebuffer backends map them via framebuffer_color_palette[].
typedef enum {
    CONSOLE_COLOR_BLACK = 0,
    CONSOLE_COLOR_BLUE,
    CONSOLE_COLOR_GREEN,
    CONSOLE_COLOR_CYAN,
    CONSOLE_COLOR_RED,
    CONSOLE_COLOR_MAGENTA,
    CONSOLE_COLOR_BROWN,
    CONSOLE_COLOR_LIGHT_GREY,
    CONSOLE_COLOR_DARK_GREY,
    CONSOLE_COLOR_LIGHT_BLUE,
    CONSOLE_COLOR_LIGHT_GREEN,
    CONSOLE_COLOR_LIGHT_CYAN,
    CONSOLE_COLOR_LIGHT_RED,
    CONSOLE_COLOR_LIGHT_MAGENTA,
    CONSOLE_COLOR_YELLOW,
    CONSOLE_COLOR_WHITE,
} console_color_t;

/// @brief Initialize the text console.
///
/// Calls arch_console_probe() to select and initialize the appropriate
/// hardware backend for the given display configuration.
///
/// @param cfg  Display configuration from the boot context.
/// @return JANUS_OK on success, JANUS_ENODEV if no console is available.
error_t drivers_console_init(display_info_t const * cfg);

/// @brief Write a single character at the current cursor position.
///
/// Interprets \\n, \\r, \\t, \\b.  Scrolls automatically when the cursor
/// reaches the bottom of the screen.
///
/// @param c  The character to write.
void drivers_console_putc(char c);

/// @brief Write a null-terminated string.
///
/// @param str  The string to write.
void drivers_console_puts(char const * str);

/// @brief Set foreground and background colors.
///
/// @param fg  Foreground color.
/// @param bg  Background color.
void drivers_console_set_color(console_color_t fg, console_color_t bg);

/// @brief Clear the screen and reset the cursor to (0, 0).
void drivers_console_clear(void);

/// @brief Set the cursor position.
///
/// @param x  Column (clamped to screen width).
/// @param y  Row (clamped to screen height).
void drivers_console_set_cursor(u16 x, u16 y);

/// @brief Get the current cursor position.
///
/// @param x  Output: current column (may be NULL).
/// @param y  Output: current row (may be NULL).
void drivers_console_get_cursor(u16 * x, u16 * y);

/// @brief Get the screen size in character cells.
///
/// @param width   Output: columns (may be NULL).
/// @param height  Output: rows (may be NULL).
void drivers_console_get_size(u16 * width, u16 * height);

#endif /* DRIVERS_CONSOLE_H */
