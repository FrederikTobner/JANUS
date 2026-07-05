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

/// @file arch/x86_64/drivers/vga_console.c
/// @brief VGA text-mode console backend implementation.

#include <arch/drivers/console.h>
#include <arch/internal/drivers/vga.h>
#include <arch/internal/drivers/vga_console.h>
#include <janus/attributes.h>

// ---------------------------------------------------------------------------
// Module state
// ---------------------------------------------------------------------------

static u16 volatile * g_vga;

// ---------------------------------------------------------------------------
// Forward declarations
// ---------------------------------------------------------------------------

static void vga_put_cell(u16 x, u16 y, char c, u8 fg, u8 bg);
static void vga_scroll_op(u8 fg, u8 bg);
static void vga_clear_op(u8 fg, u8 bg);
static void vga_set_cursor_op(u16 x, u16 y);
static void vga_get_size(u16 * w, u16 * h);

// ---------------------------------------------------------------------------
// Public functions
// ---------------------------------------------------------------------------

static console_ops_t const vga_console_backend = {
    vga_put_cell,
    vga_scroll_op,
    vga_clear_op,
    vga_set_cursor_op,
    vga_get_size,
};

__cold console_ops_t const * vga_console_init(__unused display_info_t const * cfg)
{
    g_vga = (u16 volatile *) VGA_BUFFER_PHYS;
    return &vga_console_backend;
}

// ---------------------------------------------------------------------------
// Static backend ops
// ---------------------------------------------------------------------------

static void vga_put_cell(u16 x, u16 y, char c, u8 fg, u8 bg)
{
    vga_write_cell(g_vga, x, y, c, fg, bg);
}

static void vga_scroll_op(u8 fg, u8 bg)
{
    vga_scroll(g_vga, fg, bg);
}

static void vga_clear_op(u8 fg, u8 bg)
{
    vga_clear(g_vga, fg, bg);
}

static void vga_set_cursor_op(u16 x, u16 y)
{
    vga_set_cursor(x, y);
}

static void vga_get_size(u16 * w, u16 * h)
{
    if (w) {
        *w = VGA_WIDTH;
    }
    if (h) {
        *h = VGA_HEIGHT;
    }
}
