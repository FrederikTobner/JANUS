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

#ifndef ARCH_SHARED_DRIVERS_FRAMEBUFFER_H
#define ARCH_SHARED_DRIVERS_FRAMEBUFFER_H

/// @file arch/shared/drivers/framebuffer.h
/// @brief Shared framebuffer text-rendering interface.
///
/// Provides character drawing on a linear framebuffer using the Terminus
/// 16×32 bitmap font.  Pixel primitives are delegated to lib/gfx
/// (gfx_surface_t).  This code is shared between all architectures that
/// use framebuffer output.

#include <contracts/display.h>
#include <gfx/surface.h>
#include <janus/attributes.h>
#include <janus/types.h>

#include <arch/shared/drivers/terminus.h>

#define FRAMEBUFFER_FONT_WIDTH  TERMINUS_WIDTH
#define FRAMEBUFFER_FONT_HEIGHT TERMINUS_HEIGHT

/// @brief Framebuffer state for text rendering.
///
/// Embeds a gfx_surface_t by value (public struct — no opaque handle).
/// Code that only writes pixels uses gfx_surface_t directly via the
/// gfx_surface_* primitives.
typedef struct {
    gfx_surface_t surface; ///< Pixel-level surface (lib/gfx)
    u16 text_width;        ///< Width in character cells
    u16 text_height;       ///< Height in character cells
} framebuffer_state_t;

/// @brief Standard 16-color palette (VGA-compatible).
///
/// Maps TTY color indices (0–15) to RGB values.
static u32 const framebuffer_color_palette[16] = {
    0x000000, // Black
    0x0000AA, // Blue
    0x00AA00, // Green
    0x00AAAA, // Cyan
    0xAA0000, // Red
    0xAA00AA, // Magenta
    0xAA5500, // Brown
    0xAAAAAA, // Light Grey
    0x555555, // Dark Grey
    0x5555FF, // Light Blue
    0x55FF55, // Light Green
    0x55FFFF, // Light Cyan
    0xFF5555, // Light Red
    0xFF55FF, // Light Magenta
    0xFFFF55, // Yellow
    0xFFFFFF, // White
};

/// @brief Initialize framebuffer state from a display_info_t descriptor.
///
/// @param state  Framebuffer state to initialize.
/// @param info   Display configuration from the boot context.
static inline void framebuffer_init(framebuffer_state_t * state, display_info_t const * info)
{
    gfx_surface_init(&state->surface,
                     info->framebuffer,
                     (u32) info->width,
                     (u32) info->height,
                     (u32) info->pitch,
                     info->bpp,
                     info->red_mask_shift,
                     info->green_mask_shift,
                     info->blue_mask_shift);
    state->text_width = (u16) (info->width / FRAMEBUFFER_FONT_WIDTH);
    state->text_height = (u16) (info->height / FRAMEBUFFER_FONT_HEIGHT);
}

/// @brief Draw a character at the specified text-cell position.
///
/// @param state       Framebuffer state.
/// @param column      Column (character cell, 0-based).
/// @param row         Row (character cell, 0-based).
/// @param c           Character to draw.
/// @param foreground  Foreground color index (0–15).
/// @param background  Background color index (0–15).
void framebuffer_draw_char(
    framebuffer_state_t const * state, u16 column, u16 row, char c, u8 foreground, u8 background);

#endif /* ARCH_SHARED_DRIVERS_FRAMEBUFFER_H */
