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

/// @file tty.c
/// @brief Shared logic of the TTY driver, like the cursor tracking or scrolling
///
/// Handles cursor tracking, scrolling, and color management.
/// Calls arch_tty_* functions for architecture-specific cell writing and cursor control.

#include <arch/drivers/tty.h>
#include <drivers/tty.h>
#include <janus/attributes.h>
#include <janus/errno.h>

typedef struct {
    char ch;
    u8 fg;
    u8 bg;
} tty_cell_t;

#define TTY_MAX_WIDTH  256
#define TTY_MAX_HEIGHT 128

typedef struct {
    u16 cursor_x;
    u16 cursor_y;
    u16 screen_width;
    u16 screen_height;
    u8 current_fg;
    u8 current_bg;
    tty_cell_t buffer[TTY_MAX_HEIGHT][TTY_MAX_WIDTH];
} tty_state_t;

static tty_state_t g_tty;

static void tty_scroll(void);
static void tty_set_cell(u16 x, u16 y, char ch, u8 fg, u8 bg);

__cold error_t drivers_tty_init(display_info_t const * config)
{
    error_t err = arch_tty_init(config);
    if (err != JANUS_OK) {
        return err;
    }

    arch_tty_get_size(&g_tty.screen_width, &g_tty.screen_height);
    g_tty.cursor_x = 0;
    g_tty.cursor_y = 0;
    g_tty.current_fg = TTY_COLOR_WHITE;
    g_tty.current_bg = TTY_COLOR_BLACK;

    drivers_tty_clear();
    return JANUS_OK;
}

__hot void drivers_tty_putc(char c)
{
    if (c == '\n') {
        g_tty.cursor_x = 0;
        g_tty.cursor_y++;
    } else if (c == '\r') {
        g_tty.cursor_x = 0;
    } else if (c == '\t') {
        g_tty.cursor_x = (g_tty.cursor_x + 8) & ~7;
    } else if (c == '\b') {
        if (g_tty.cursor_x > 0) {
            g_tty.cursor_x--;
        }
    } else {
        tty_cell_t cell = {.ch = c, .fg = g_tty.current_fg, .bg = g_tty.current_bg};
        tty_set_cell(g_tty.cursor_x, g_tty.cursor_y, cell.ch, cell.fg, cell.bg);
        g_tty.cursor_x++;
    }

    // Handle line wrap
    if (g_tty.cursor_x >= g_tty.screen_width) {
        g_tty.cursor_x = 0;
        g_tty.cursor_y++;
    }

    // Handle scroll
    if (g_tty.cursor_y >= g_tty.screen_height) {
        tty_scroll();
        g_tty.cursor_y = g_tty.screen_height - 1;
    }

    arch_tty_set_cursor(g_tty.cursor_x, g_tty.cursor_y);
}

void drivers_tty_puts(char const * str)
{
    while (*str) {
        drivers_tty_putc(*str++);
    }
}

void drivers_tty_set_color(tty_color_t fg, tty_color_t bg)
{
    g_tty.current_fg = (u8) fg;
    g_tty.current_bg = (u8) bg;
}

void drivers_tty_clear(void)
{
    for (u16 y = 0; y < g_tty.screen_height; y++) {
        for (u16 x = 0; x < g_tty.screen_width; x++) {
            tty_set_cell(x, y, ' ', g_tty.current_fg, g_tty.current_bg);
        }
    }
    g_tty.cursor_x = 0;
    g_tty.cursor_y = 0;
    arch_tty_set_cursor(0, 0);
}

void drivers_tty_set_cursor(u16 x, u16 y)
{
    if (g_tty.screen_width == 0 || g_tty.screen_height == 0) {
        return; // Screen size not initialized
    }
    g_tty.cursor_x = (x < g_tty.screen_width) ? x : g_tty.screen_width - 1;
    g_tty.cursor_y = (y < g_tty.screen_height) ? y : g_tty.screen_height - 1;
    arch_tty_set_cursor(g_tty.cursor_x, g_tty.cursor_y);
}

void drivers_tty_get_cursor(u16 * x, u16 * y)
{
    if (x) {
        *x = g_tty.cursor_x;
    }
    if (y) {
        *y = g_tty.cursor_y;
    }
}

void drivers_tty_get_size(u16 * width, u16 * height)
{
    if (width) {
        *width = g_tty.screen_width;
    }
    if (height) {
        *height = g_tty.screen_height;
    }
}

static void tty_scroll(void)
{
    // Move all lines up by one
    for (u16 y = 0; y < g_tty.screen_height - 1; y++) {
        for (u16 x = 0; x < g_tty.screen_width; x++) {
            tty_cell_t c = g_tty.buffer[y + 1][x];
            tty_set_cell(x, y, c.ch, c.fg, c.bg); // Clear old cell
        }
    }
    // Clear last line
    u16 const last_line = g_tty.screen_height - 1;
    for (u16 x = 0; x < g_tty.screen_width; x++) {
        tty_set_cell(x, last_line, ' ', g_tty.current_fg, g_tty.current_bg);
    }
}

static void tty_set_cell(u16 x, u16 y, char ch, u8 fg, u8 bg)
{
    if (x < g_tty.screen_width && y < g_tty.screen_height) {
        arch_tty_write_cell(x, y, ch, fg, bg);
    }
}