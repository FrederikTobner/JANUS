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

/// @file gfx/draw.c
/// @brief Stateless pixel primitives — implementation.

#include <gfx/draw.h>
#include <gfx/surface.h>
#include <janus/attributes.h>

static void write_pixel(gfx_surface_t const * s, u32 x, u32 y, u32 rgb);

__cold void gfx_surface_init(
    gfx_surface_t * s, void * base, u32 width, u32 height, u32 pitch, u16 bpp, u8 r_shift, u8 g_shift, u8 b_shift)
{
    s->base = (u8 volatile *) base;
    s->width = width;
    s->height = height;
    s->pitch = pitch;
    s->bpp = bpp;
    s->red_shift = r_shift;
    s->green_shift = g_shift;
    s->blue_shift = b_shift;
}

__hot void gfx_surface_put_pixel(gfx_surface_t const * s, u32 x, u32 y, u32 rgb)
{
    if (UNLIKELY(!s->base || x >= s->width || y >= s->height)) {
        return;
    }
    write_pixel(s, x, y, rgb);
}

__hot void gfx_surface_fill_rect(gfx_surface_t const * s, u32 x, u32 y, u32 w, u32 h, u32 rgb)
{
    if (UNLIKELY(!s->base)) {
        return;
    }
    // Clip to surface bounds. Widen to u64 before adding to avoid u32 overflow.
    u64 x_limit = (u64) x + (u64) w;
    u64 y_limit = (u64) y + (u64) h;
    u32 x_end = (x_limit < (u64) s->width) ? (u32) x_limit : s->width;
    u32 y_end = (y_limit < (u64) s->height) ? (u32) y_limit : s->height;
    if (x >= x_end || y >= y_end) {
        return;
    }
    for (u32 row = y; row < y_end; row++) {
        for (u32 col = x; col < x_end; col++) {
            write_pixel(s, col, row, rgb);
        }
    }
}

__hot void gfx_surface_blit_mono(
    gfx_surface_t const * s, u32 x, u32 y, u8 const * bitmap, u32 width, u32 height, u32 fg_rgb, u32 bg_rgb)
{
    if (UNLIKELY(!s->base || !bitmap)) {
        return;
    }
    if (UNLIKELY(width == 0 || width % 8 != 0)) {
        return;
    }
    u32 bytes_per_row = width / 8;
    for (u32 row = 0; row < height; row++) {
        for (u32 byte_idx = 0; byte_idx < bytes_per_row; byte_idx++) {
            u8 bits = bitmap[(row * bytes_per_row) + byte_idx];
            for (u32 bit = 0; bit < 8; bit++) {
                u32 px = x + (byte_idx * 8) + bit;
                u32 py = y + row;
                u32 color = (bits & (0x80U >> bit)) ? fg_rgb : bg_rgb;
                if (px < s->width && py < s->height) {
                    write_pixel(s, px, py, color);
                }
            }
        }
    }
}

__hot void gfx_surface_scroll(gfx_surface_t const * s, u32 dy, u32 fill_rgb)
{
    if (UNLIKELY(!s->base)) {
        return;
    }
    if (dy >= s->height) {
        gfx_surface_fill_rect(s, 0, 0, s->width, s->height, fill_rgb);
        return;
    }
    // Copy scanlines [dy, height) upward to [0, height-dy).
    // Ascending iteration is safe: destination row y < source row y+dy.
    u32 const copy_rows = s->height - dy;
    for (u32 row = 0; row < copy_rows; row++) {
        u8 volatile * dst = s->base + (u64) row * s->pitch;
        u8 const volatile * src = s->base + (u64) (row + dy) * s->pitch;
        for (u32 i = 0; i < s->pitch; i++) {
            dst[i] = src[i];
        }
    }
    // Fill the newly exposed bottom rows.
    gfx_surface_fill_rect(s, 0, s->height - dy, s->width, dy, fill_rgb);
}

// ---------------------------------------------------------------------------
// Static helpers
// ---------------------------------------------------------------------------

static void write_pixel(gfx_surface_t const * s, u32 x, u32 y, u32 rgb)
{
    if (UNLIKELY(s->bpp != 32 && s->bpp != 24)) {
        return;
    }
    // Widen to u64 before multiplying to avoid u32 overflow on large pitches.
    u64 offset = ((u64) y * (u64) s->pitch) + ((u64) x * (u64) (s->bpp / 8));
    u32 pixel = (u32) (((rgb >> 16) & 0xFF) << s->red_shift) | (u32) (((rgb >> 8) & 0xFF) << s->green_shift) |
                (u32) ((rgb & 0xFF) << s->blue_shift);

    if (s->bpp == 32) {
        *((u32 volatile *) (s->base + offset)) = pixel;
    } else {
        // 24 bpp — three individual byte writes, little-endian order.
        s->base[offset] = (u8) (pixel & 0xFF);
        s->base[offset + 1] = (u8) ((pixel >> 8) & 0xFF);
        s->base[offset + 2] = (u8) ((pixel >> 16) & 0xFF);
    }
}
