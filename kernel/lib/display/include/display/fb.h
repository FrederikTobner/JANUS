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

#ifndef DISPLAY_FB_H
#define DISPLAY_FB_H

/// @file display/fb.h
/// @brief Stateless linear framebuffer pixel primitives.
///
/// Architecture-independent pixel operations on a linear RGB framebuffer.
/// These functions are stateless: they take an explicit display_fb_t handle
/// and perform no cursor tracking or text-mode logic. Callers that need
/// text state (e.g. the TTY driver) should build their own wrapper.
///
/// Supports 24 bpp (3 bytes per pixel) and 32 bpp (4 bytes per pixel).

#include <janus/attributes.h>
#include <janus/types.h>

/// @brief Minimal view of a linear framebuffer — enough to write pixels.
///
/// Constructed by the consumer (e.g. @c framebuffer_init in the drivers
/// subsystem) from @c display_info_t. Does not own the memory it points to.
typedef struct {
    u8 volatile * base; ///< Framebuffer base address
    u64 pitch;          ///< Bytes per scanline
    u64 width;          ///< Horizontal resolution in pixels
    u64 height;         ///< Vertical resolution in pixels
    u8 red_shift;       ///< Bit position of the red channel
    u8 green_shift;     ///< Bit position of the green channel
    u8 blue_shift;      ///< Bit position of the blue channel
    u16 bpp;            ///< Bits per pixel (24 or 32)
} display_fb_t;

/// @brief Write a single pixel at (x, y).
///
/// @param fb     Framebuffer handle.
/// @param x      X coordinate in pixels (0 = left).
/// @param y      Y coordinate in pixels (0 = top).
/// @param rgb    Color as 0x00RRGGBB.
void display_put_pixel(display_fb_t const * fb, u32 x, u32 y, u32 rgb);

/// @brief Fill an axis-aligned rectangle with a solid color.
///
/// The rectangle is clipped to the framebuffer bounds.
///
/// @param fb     Framebuffer handle.
/// @param x      Left edge in pixels.
/// @param y      Top edge in pixels.
/// @param w      Width in pixels.
/// @param h      Height in pixels.
/// @param rgb    Fill color as 0x00RRGGBB.
void display_fill_rect(display_fb_t const * fb, u32 x, u32 y, u32 w, u32 h, u32 rgb);

/// @brief Blit a monochrome glyph bitmap onto the framebuffer.
///
/// The bitmap is stored as rows of @p width pixels, MSB first, one byte per
/// 8-pixel group. A set bit draws @p fg_rgb; a clear bit draws @p bg_rgb.
/// This matches the PSF2 / Terminus font storage format used by psf2c.
///
/// @param fb      Framebuffer handle.
/// @param x       Left edge of the glyph in pixels.
/// @param y       Top edge of the glyph in pixels.
/// @param bitmap  Glyph bitmap data.
/// @param width   Glyph width in pixels (must be a multiple of 8).
/// @param height  Glyph height in pixels.
/// @param fg_rgb  Foreground color as 0x00RRGGBB.
/// @param bg_rgb  Background color as 0x00RRGGBB.
void display_blit_glyph(
    display_fb_t const * fb, u32 x, u32 y, u8 const * bitmap, u32 width, u32 height, u32 fg_rgb, u32 bg_rgb);

#endif /* DISPLAY_FB_H */
