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

/// @file display/fb.c
/// @brief Stateless linear framebuffer pixel primitives — implementation.

#include <display/fb.h>
#include <janus/attributes.h>

// --------------------------------------------------------------------------
// Struct definition (private — not exposed in fb.h)
// --------------------------------------------------------------------------

struct display_fb {
    u8 volatile * base; ///< Framebuffer base address
    u64 pitch;          ///< Bytes per scanline
    u64 width;          ///< Horizontal resolution in pixels
    u64 height;         ///< Vertical resolution in pixels
    u8 red_shift;       ///< Bit position of the red channel
    u8 green_shift;     ///< Bit position of the green channel
    u8 blue_shift;      ///< Bit position of the blue channel
    u16 bpp;            ///< Bits per pixel (24 or 32)
};

static display_fb_t g_display_fb;

// --------------------------------------------------------------------------
// Constructor
// --------------------------------------------------------------------------

__cold display_fb_t *
display_fb_init(void * base, u64 width, u64 height, u64 pitch, u16 bpp, u8 r_shift, u8 g_shift, u8 b_shift)
{
    g_display_fb.base = (u8 volatile *) base;
    g_display_fb.width = width;
    g_display_fb.height = height;
    g_display_fb.pitch = pitch;
    g_display_fb.bpp = bpp;
    g_display_fb.red_shift = r_shift;
    g_display_fb.green_shift = g_shift;
    g_display_fb.blue_shift = b_shift;
    return &g_display_fb;
}

// --------------------------------------------------------------------------
// Internal helper: write one pixel
// --------------------------------------------------------------------------

static __always_inline void write_pixel(display_fb_t const * fb, u32 x, u32 y, u32 rgb)
{
    if (UNLIKELY(fb->bpp != 32 && fb->bpp != 24)) {
        return;
    }
    u64 offset = ((u64) y * fb->pitch) + ((u64) x * (fb->bpp / 8));
    u32 pixel = (u32) (((rgb >> 16) & 0xFF) << fb->red_shift) | (u32) (((rgb >> 8) & 0xFF) << fb->green_shift) |
                (u32) ((rgb & 0xFF) << fb->blue_shift);

    if (fb->bpp == 32) {
        *((u32 volatile *) (fb->base + offset)) = pixel;
    } else {
        // 24 bpp — three individual byte writes, little-endian order.
        fb->base[offset] = (u8) (pixel & 0xFF);
        fb->base[offset + 1] = (u8) ((pixel >> 8) & 0xFF);
        fb->base[offset + 2] = (u8) ((pixel >> 16) & 0xFF);
    }
}

// --------------------------------------------------------------------------
// Public API
// --------------------------------------------------------------------------

__hot void display_put_pixel(display_fb_t const * fb, u32 x, u32 y, u32 rgb)
{
    if (UNLIKELY(!fb->base || x >= (u32) fb->width || y >= (u32) fb->height)) {
        return;
    }
    write_pixel(fb, x, y, rgb);
}

__hot void display_fill_rect(display_fb_t const * fb, u32 x, u32 y, u32 w, u32 h, u32 rgb)
{
    if (UNLIKELY(!fb->base)) {
        return;
    }
    // Clip to framebuffer bounds. Compute ends in u64 to avoid overflow in x + w / y + h.
    u64 x_limit = (u64) x + (u64) w;
    u64 y_limit = (u64) y + (u64) h;
    u32 x_end = (x_limit < (u64) fb->width) ? (u32) x_limit : (u32) fb->width;
    u32 y_end = (y_limit < (u64) fb->height) ? (u32) y_limit : (u32) fb->height;
    if (x >= x_end || y >= y_end) {
        return;
    }
    for (u32 row = y; row < y_end; row++) {
        for (u32 col = x; col < x_end; col++) {
            write_pixel(fb, col, row, rgb);
        }
    }
}

__hot void display_blit_glyph(
    display_fb_t const * fb, u32 x, u32 y, u8 const * bitmap, u32 width, u32 height, u32 fg_rgb, u32 bg_rgb)
{
    if (UNLIKELY(!fb->base || !bitmap)) {
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
                if (px < (u32) fb->width && py < (u32) fb->height) {
                    write_pixel(fb, px, py, color);
                }
            }
        }
    }
}
