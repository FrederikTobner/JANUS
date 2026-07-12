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

/// @file arch/shared/drivers/fb_console.c
/// @brief Framebuffer text-console backend implementation.
///
/// Implements the console_ops_t interface for a linear RGB framebuffer.
/// Character drawing is delegated to framebuffer_draw_char(); scroll and
/// clear are implemented with gfx_surface_scroll / gfx_surface_fill_rect.

#include <arch/drivers/console.h>
#include <arch/shared/drivers/fb_console.h>
#include <arch/shared/drivers/framebuffer.h>
#include <gfx/draw.h>
#include <janus/attributes.h>

static framebuffer_state_t g_fb;

static void fb_put_cell(u16 x, u16 y, char c, u8 fg, u8 bg);
static void fb_scroll(u8 fg, u8 bg);
static void fb_clear(u8 fg, u8 bg);
static void fb_set_cursor(u16 x, u16 y);
static void fb_get_size(u16 * w, u16 * h);

static console_ops_t const fb_console_backend = {
    fb_put_cell,
    fb_scroll,
    fb_clear,
    fb_set_cursor,
    fb_get_size,
};

__cold console_ops_t const * fb_console_init(display_info_t const * cfg)
{
    framebuffer_init(&g_fb, cfg);
    return &fb_console_backend;
}

static void fb_put_cell(u16 x, u16 y, char c, u8 fg, u8 bg)
{
    framebuffer_draw_char(&g_fb, x, y, c, fg, bg);
}

static void fb_scroll(__unused u8 fg, u8 bg)
{
    gfx_surface_scroll(&g_fb.surface, FRAMEBUFFER_FONT_HEIGHT, framebuffer_color_palette[bg & 0x0F]);
}

static void fb_clear(__unused u8 fg, u8 bg)
{
    gfx_surface_fill_rect(&g_fb.surface,
                          0,
                          0,
                          (u32) g_fb.text_width * FRAMEBUFFER_FONT_WIDTH,
                          (u32) g_fb.text_height * FRAMEBUFFER_FONT_HEIGHT,
                          framebuffer_color_palette[bg & 0x0F]);
}

static void fb_set_cursor(__unused u16 x, __unused u16 y)
{
    // No hardware cursor on framebuffer; software cursor is a future option.
}

static void fb_get_size(u16 * w, u16 * h)
{
    if (w) {
        *w = g_fb.text_width;
    }
    if (h) {
        *h = g_fb.text_height;
    }
}
