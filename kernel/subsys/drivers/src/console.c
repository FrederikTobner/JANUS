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

/// @file drivers/console.c
/// @brief Text console terminal policy.
///
/// Handles cursor tracking, line wrap, scrolling, and character
/// classification.  Delegates all device I/O to the console_ops_t
/// returned by arch_console_probe().  No fixed-size shadow buffer —
/// device state lives in the device backend.

#include <arch/drivers/console.h>
#include <drivers/console.h>
#include <janus/attributes.h>
#include <janus/errno.h>

typedef struct {
    u16 cursor_x;
    u16 cursor_y;
    u16 screen_width;
    u16 screen_height;
    u8 current_fg;
    u8 current_bg;
} console_state_t;

static console_ops_t const * g_console;
static console_state_t g_con;

__cold error_t drivers_console_init(display_info_t const * cfg)
{
    g_console = arch_console_probe(cfg);
    if (g_console == NULL) {
        return JANUS_ENODEV;
    }
    g_console->get_size(&g_con.screen_width, &g_con.screen_height);
    g_con.cursor_x = 0;
    g_con.cursor_y = 0;
    g_con.current_fg = CONSOLE_COLOR_WHITE;
    g_con.current_bg = CONSOLE_COLOR_BLACK;
    drivers_console_clear();
    return JANUS_OK;
}

__hot void drivers_console_putc(char c)
{
    switch (c) {
    case '\n':
        g_con.cursor_x = 0;
        g_con.cursor_y++;
        break;
    case '\r':
        g_con.cursor_x = 0;
        break;
    case '\t':
        g_con.cursor_x = (u16) ((g_con.cursor_x + 8) & ~7U);
        break;
    case '\b':
        if (g_con.cursor_x > 0) {
            g_con.cursor_x--;
            g_console->put_cell(g_con.cursor_x, g_con.cursor_y, ' ', g_con.current_fg, g_con.current_bg);
        }
        break;
    default:
        g_console->put_cell(g_con.cursor_x, g_con.cursor_y, c, g_con.current_fg, g_con.current_bg);
        g_con.cursor_x++;
        break;
    }

    if (g_con.cursor_x >= g_con.screen_width) {
        g_con.cursor_x = 0;
        g_con.cursor_y++;
    }
    if (g_con.cursor_y >= g_con.screen_height) {
        g_console->scroll(g_con.current_fg, g_con.current_bg);
        g_con.cursor_y = (u16) (g_con.screen_height - 1);
    }

    g_console->set_cursor(g_con.cursor_x, g_con.cursor_y);
}

void drivers_console_puts(char const * str)
{
    while (*str) {
        drivers_console_putc(*str++);
    }
}

void drivers_console_set_color(console_color_t fg, console_color_t bg)
{
    g_con.current_fg = (u8) fg;
    g_con.current_bg = (u8) bg;
}

void drivers_console_clear(void)
{
    g_console->clear(g_con.current_fg, g_con.current_bg);
    g_con.cursor_x = 0;
    g_con.cursor_y = 0;
    g_console->set_cursor(0, 0);
}

void drivers_console_set_cursor(u16 x, u16 y)
{
    if (g_con.screen_width == 0 || g_con.screen_height == 0) {
        return;
    }
    g_con.cursor_x = (x < g_con.screen_width) ? x : (u16) (g_con.screen_width - 1);
    g_con.cursor_y = (y < g_con.screen_height) ? y : (u16) (g_con.screen_height - 1);
    g_console->set_cursor(g_con.cursor_x, g_con.cursor_y);
}

void drivers_console_get_cursor(u16 * x, u16 * y)
{
    if (x) {
        *x = g_con.cursor_x;
    }
    if (y) {
        *y = g_con.cursor_y;
    }
}

void drivers_console_get_size(u16 * width, u16 * height)
{
    if (width) {
        *width = g_con.screen_width;
    }
    if (height) {
        *height = g_con.screen_height;
    }
}
