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
 * @file tty.c
 * @brief TTY driver — shared logic (cursor, scrolling).
 *
 * Handles cursor tracking, scrolling, and color management.
 * Calls arch_tty_* functions for hardware access.
 */

#include <arch/drivers/tty.h>
#include <drivers/tty.h>
#include <janus/attributes.h>

static u16 cursor_x = 0;
static u16 cursor_y = 0;
static u16 screen_width = 0;
static u16 screen_height = 0;
static u8 current_fg = TTY_COLOR_WHITE;
static u8 current_bg = TTY_COLOR_BLACK;

static __hot void tty_scroll(void)
{
    // Move all lines up by one
    for (u16 y = 0; y < screen_height - 1; y++) {
        for (u16 x = 0; x < screen_width; x++) {
            char ch;
            u8 fg, bg;
            arch_tty_read_cell(x, y + 1, &ch, &fg, &bg);
            arch_tty_write_cell(x, y, ch, fg, bg);
        }
    }
    // Clear last line
    for (u16 x = 0; x < screen_width; x++) {
        arch_tty_write_cell(x, screen_height - 1, ' ', current_fg, current_bg);
    }
}

__cold error_t drivers_tty_init(display_info_t const * config)
{
    error_t err = arch_tty_init(config);
    if (err != 0) {
        return err;
    }

    arch_tty_get_size(&screen_width, &screen_height);
    cursor_x = 0;
    cursor_y = 0;
    current_fg = TTY_COLOR_WHITE;
    current_bg = TTY_COLOR_BLACK;

    drivers_tty_clear();
    return 0;
}

__hot void drivers_tty_putc(char c)
{
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\t') {
        cursor_x = (cursor_x + 8) & ~7;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
        }
    } else {
        arch_tty_write_cell(cursor_x, cursor_y, c, current_fg, current_bg);
        cursor_x++;
    }

    /* Handle line wrap */
    if (cursor_x >= screen_width) {
        cursor_x = 0;
        cursor_y++;
    }

    /* Handle scroll */
    if (cursor_y >= screen_height) {
        tty_scroll();
        cursor_y = screen_height - 1;
    }

    arch_tty_set_cursor(cursor_x, cursor_y);
}

void drivers_tty_puts(char const * str)
{
    while (*str) {
        drivers_tty_putc(*str++);
    }
}

void drivers_tty_set_color(tty_color_t fg, tty_color_t bg)
{
    current_fg = (u8) fg;
    current_bg = (u8) bg;
}

void drivers_tty_clear(void)
{
    for (u16 y = 0; y < screen_height; y++) {
        for (u16 x = 0; x < screen_width; x++) {
            arch_tty_write_cell(x, y, ' ', current_fg, current_bg);
        }
    }
    cursor_x = 0;
    cursor_y = 0;
    arch_tty_set_cursor(0, 0);
}

void drivers_tty_set_cursor(u16 x, u16 y)
{
    if (screen_width == 0 || screen_height == 0) {
        return; // Screen size not initialized
    }
    cursor_x = (x < screen_width) ? x : screen_width - 1;
    cursor_y = (y < screen_height) ? y : screen_height - 1;
    arch_tty_set_cursor(cursor_x, cursor_y);
}

void drivers_tty_get_cursor(u16 * x, u16 * y)
{
    if (x) {
        *x = cursor_x;
    }
    if (y) {
        *y = cursor_y;
    }
}

void drivers_tty_get_size(u16 * width, u16 * height)
{
    if (width) {
        *width = screen_width;
    }
    if (height) {
        *height = screen_height;
    }
}
