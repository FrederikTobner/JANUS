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

/// @file tty.h
/// @brief Text display driver interface.
///
/// Architecture contract (arch_tty_*) is in <arch/drivers/tty.h>.
/// Shared logic (cursor tracking, scrolling) is in tty.c.
///
/// Backends:
/// - x86_64: VGA text mode via port I/O or framebuffer text rendering via memory-mapped I/O
/// - aarch64: Framebuffer text rendering

#ifndef DRIVERS_TTY_H
#define DRIVERS_TTY_H

#include <display/display.h>
#include <janus/types.h>

/// @brief Logical TTY text color indices (16-color ANSI-compatible palette).
///
/// Values are architecture-agnostic sequential indices 0–15.
/// x86_64 VGA text mode maps these directly to hardware nibble values (same numbering).
/// Framebuffer backends map them via @c framebuffer_color_palette[].
/// If a future backend uses different hardware values, add an arch-specific
/// translation function rather than changing these constants.
typedef enum {
    TTY_COLOR_BLACK = 0,
    TTY_COLOR_BLUE,
    TTY_COLOR_GREEN,
    TTY_COLOR_CYAN,
    TTY_COLOR_RED,
    TTY_COLOR_MAGENTA,
    TTY_COLOR_BROWN,
    TTY_COLOR_LIGHT_GREY,
    TTY_COLOR_DARK_GREY,
    TTY_COLOR_LIGHT_BLUE,
    TTY_COLOR_LIGHT_GREEN,
    TTY_COLOR_LIGHT_CYAN,
    TTY_COLOR_LIGHT_RED,
    TTY_COLOR_LIGHT_MAGENTA,
    TTY_COLOR_YELLOW,
    TTY_COLOR_WHITE,
} tty_color_t;

/// @brief Initialize the TTY.
/// @param config Display configuration. The @c mode field determines the backend:
///               DISPLAY_MODE_FRAMEBUFFER uses framebuffer text rendering,
///               DISPLAY_MODE_VGA_TEXT uses VGA hardware (x86_64 only).
/// @return 0 on success, negative error code if not available.
error_t drivers_tty_init(display_info_t const * config);

/// @brief Write a single character at cursor position.
/// @param c The character to write.
void drivers_tty_putc(char c);

/// @brief Write a null-terminated string.
/// @param str The string to write.
void drivers_tty_puts(char const * str);

/// @brief Set foreground and background colors.
/// @param fg Foreground color.
/// @param bg Background color.
void drivers_tty_set_color(tty_color_t fg, tty_color_t bg);

/// @brief Clear the screen.
void drivers_tty_clear(void);

/// @brief Set cursor position.
/// @param x Column position.
/// @param y Row position.
void drivers_tty_set_cursor(u16 x, u16 y);

/// @brief Get cursor position.
/// @param x Pointer to store column (may be NULL).
/// @param y Pointer to store row (may be NULL).
void drivers_tty_get_cursor(u16 * x, u16 * y);

/// @brief Get screen dimensions.
/// @param width Pointer to store width (may be NULL).
/// @param height Pointer to store height (may be NULL).
void drivers_tty_get_size(u16 * width, u16 * height);

#endif /* DRIVERS_TTY_H */
