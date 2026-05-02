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

/// @file framebuffer.c
/// @brief Shared framebuffer text rendering implementation.
///
/// Contains the character drawing logic for framebuffer-based text output.
/// Pixel operations are delegated to lib/display (display_blit_glyph).
/// This code is shared between architectures (x86_64 Limine mode, aarch64).

#include <arch/shared/drivers/framebuffer.h>

__cold void
framebuffer_draw_char(framebuffer_state_t const * state, u16 column, u16 row, char c, u8 foreground, u8 background)
{
    if (UNLIKELY(!state->fb || column >= state->text_width || row >= state->text_height)) {
        return;
    }

    u32 px = (u32) column * FRAMEBUFFER_FONT_WIDTH;
    u32 py = (u32) row * FRAMEBUFFER_FONT_HEIGHT;
    u32 fg = framebuffer_color_palette[foreground & 0x0F];
    u32 bg = framebuffer_color_palette[background & 0x0F];

    display_blit_glyph(
        state->fb, px, py, terminus_glyphs[(u8) c], FRAMEBUFFER_FONT_WIDTH, FRAMEBUFFER_FONT_HEIGHT, fg, bg);
}
