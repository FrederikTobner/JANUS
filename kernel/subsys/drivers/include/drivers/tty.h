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

/**
 * @file tty.h
 * @brief Text display driver interface.
 *
 * This header contains:
 * - Public API (tty_*)
 * - Types (tty_color_t)
 *
 * Architecture contract (arch_tty_*) is in <arch/drivers/tty.h>.
 * Shared logic (cursor tracking, scrolling) is in tty.c.
 *
 * Backends:
 * - x86_64: VGA text mode (0xB8000)
 * - aarch64: Framebuffer text rendering (future)
 */

#ifndef DRIVERS_TTY_H
#define DRIVERS_TTY_H

#include <janus/types.h>

/*===========================================================================
 * Types
 *===========================================================================*/

/** TTY text colors (portable subset) */
typedef enum {
    TTY_COLOR_BLACK = 0,
    TTY_COLOR_BLUE = 1,
    TTY_COLOR_GREEN = 2,
    TTY_COLOR_CYAN = 3,
    TTY_COLOR_RED = 4,
    TTY_COLOR_MAGENTA = 5,
    TTY_COLOR_BROWN = 6,
    TTY_COLOR_LIGHT_GREY = 7,
    TTY_COLOR_DARK_GREY = 8,
    TTY_COLOR_LIGHT_BLUE = 9,
    TTY_COLOR_LIGHT_GREEN = 10,
    TTY_COLOR_LIGHT_CYAN = 11,
    TTY_COLOR_LIGHT_RED = 12,
    TTY_COLOR_LIGHT_MAGENTA = 13,
    TTY_COLOR_YELLOW = 14,
    TTY_COLOR_WHITE = 15,
} tty_color_t;

/*===========================================================================
 * Public API — Implemented in subsys/drivers/tty.c
 *===========================================================================*/

/**
 * @brief Initialize the TTY.
 * @return 0 on success, negative error code if not available.
 */
error_t tty_init(void);

/**
 * @brief Write a single character at cursor position.
 * @param c The character to write.
 */
void tty_putc(char c);

/**
 * @brief Write a null-terminated string.
 * @param str The string to write.
 */
void tty_puts(char const * str);

/**
 * @brief Set foreground and background colors.
 * @param fg Foreground color.
 * @param bg Background color.
 */
void tty_set_color(tty_color_t fg, tty_color_t bg);

/**
 * @brief Clear the screen.
 */
void tty_clear(void);

/**
 * @brief Set cursor position.
 * @param x Column position.
 * @param y Row position.
 */
void tty_set_cursor(u16 x, u16 y);

/**
 * @brief Get cursor position.
 * @param x Pointer to store column (may be NULL).
 * @param y Pointer to store row (may be NULL).
 */
void tty_get_cursor(u16 * x, u16 * y);

/**
 * @brief Get screen dimensions.
 * @param width Pointer to store width (may be NULL).
 * @param height Pointer to store height (may be NULL).
 */
void tty_get_size(u16 * width, u16 * height);

#endif /* DRIVERS_TTY_H */
