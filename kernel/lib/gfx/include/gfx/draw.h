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

#ifndef GFX_DRAW_H
#define GFX_DRAW_H

/// @file gfx/draw.h
/// @brief Stateless pixel primitives for a gfx_surface_t.
///
/// All functions are stateless: they take an explicit gfx_surface_t and
/// perform no cursor tracking or text-mode logic.
/// Supports 24 bpp (3 bytes per pixel) and 32 bpp (4 bytes per pixel).

#include <gfx/surface.h>
#include <janus/attributes.h>
#include <janus/types.h>

/// @brief Write a single pixel at (x, y).
///
/// @param s    Target surface.
/// @param x    X coordinate in pixels (0 = left).
/// @param y    Y coordinate in pixels (0 = top).
/// @param rgb  Color as 0x00RRGGBB.
void gfx_surface_put_pixel(gfx_surface_t const * s, u32 x, u32 y, u32 rgb);

/// @brief Fill an axis-aligned rectangle with a solid color.
///
/// The rectangle is clipped to the surface bounds.
///
/// @param s    Target surface.
/// @param x    Left edge in pixels.
/// @param y    Top edge in pixels.
/// @param w    Width in pixels.
/// @param h    Height in pixels.
/// @param rgb  Fill color as 0x00RRGGBB.
void gfx_surface_fill_rect(gfx_surface_t const * s, u32 x, u32 y, u32 w, u32 h, u32 rgb);

/// @brief Blit a monochrome 1-bpp bitmap onto the surface.
///
/// Pixels are stored MSB-first, one byte per 8-pixel group.
/// @p width must be a non-zero multiple of 8.
/// Set bits draw @p fg_rgb; clear bits draw @p bg_rgb.
///
/// @param s        Target surface.
/// @param x        Left edge in pixels.
/// @param y        Top edge in pixels.
/// @param bitmap   Row-major 1-bpp source data.
/// @param width    Bitmap width in pixels (non-zero, multiple of 8).
/// @param height   Bitmap height in pixels.
/// @param fg_rgb   Foreground color as 0x00RRGGBB.
/// @param bg_rgb   Background color as 0x00RRGGBB.
void gfx_surface_blit_mono(
    gfx_surface_t const * s, u32 x, u32 y, u8 const * bitmap, u32 width, u32 height, u32 fg_rgb, u32 bg_rgb);

/// @brief Scroll the surface up by @p dy pixel rows.
///
/// Copies scanlines [dy, height) upward to [0, height-dy), then fills
/// [height-dy, height) with @p fill_rgb. If dy >= height, the entire
/// surface is filled.
///
/// @param s         Target surface.
/// @param dy        Number of pixel rows to scroll up.
/// @param fill_rgb  Fill color for the newly exposed rows (0x00RRGGBB).
void gfx_surface_scroll(gfx_surface_t const * s, u32 dy, u32 fill_rgb);

#endif /* GFX_DRAW_H */
