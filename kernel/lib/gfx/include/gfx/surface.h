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

#ifndef GFX_SURFACE_H
#define GFX_SURFACE_H

/// @file gfx/surface.h
/// @brief Drawable pixel-surface type and initializer.
///
/// gfx_surface_t is a public struct — callers embed it by value and own
/// their own storage. Use gfx_surface_init() to populate the fields.
/// Pixel drawing operations are declared in <gfx/draw.h>.

#include <janus/types.h>

/// @brief A drawable linear-RGB pixel surface.
///
/// Backed by a framebuffer today; may back an off-screen buffer later.
/// The struct is intentionally public so callers can embed surfaces by
/// value, enabling multiple surfaces without heap allocation.
typedef struct {
    u8 volatile * base; ///< Pixel-plane base address
    u32 width;          ///< Width in pixels
    u32 height;         ///< Height in pixels
    u32 pitch;          ///< Bytes per scanline
    u16 bpp;            ///< Bits per pixel (24 or 32)
    u8 red_shift;       ///< Red channel bit position
    u8 green_shift;     ///< Green channel bit position
    u8 blue_shift;      ///< Blue channel bit position
} gfx_surface_t;

/// @brief Initialize a caller-owned surface.
///
/// No allocation; no singleton. The caller provides the storage.
/// Narrowing from display_info_t's u64 geometry to u32 happens here —
/// casts are explicit to satisfy -Wconversion.
///
/// @param s        Surface to initialize.
/// @param base     Framebuffer base address.
/// @param width    Horizontal resolution in pixels.
/// @param height   Vertical resolution in pixels.
/// @param pitch    Bytes per scanline.
/// @param bpp      Bits per pixel (24 or 32).
/// @param r_shift  Red channel bit position.
/// @param g_shift  Green channel bit position.
/// @param b_shift  Blue channel bit position.
void gfx_surface_init(
    gfx_surface_t * s, void * base, u32 width, u32 height, u32 pitch, u16 bpp, u8 r_shift, u8 g_shift, u8 b_shift);

#endif /* GFX_SURFACE_H */
