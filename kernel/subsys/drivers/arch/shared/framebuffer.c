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

/**
 * @file framebuffer.c
 * @brief Shared framebuffer text rendering implementation.
 *
 * Contains the character drawing logic for framebuffer-based text output.
 * This code is shared between architectures (x86_64 Limine mode, aarch64).
 */

#include <arch/shared/drivers/framebuffer.h>

__hot void
framebuffer_draw_char(framebuffer_state_t const * state, u16 column, u16 row, char c, u8 foreground, u8 background)
{
    if (UNLIKELY(!state->base || column >= state->text_width || row >= state->text_height)) {
        return;
    }

    u64 px = column * FRAMEBUFFER_FONT_WIDTH;
    u64 py = row * FRAMEBUFFER_FONT_HEIGHT;
    u32 fg_color = framebuffer_color_palette[foreground & 0x0F];
    u32 bg_color = framebuffer_color_palette[background & 0x0F];

    // Get font bitmap for this character (Terminus font covers all 256 codepoints)
    u8 const * glyph = terminus_glyphs[(u8) c];

    // Draw each pixel of the character.
    // Terminus 16x32 is stored as 2 bytes per row (16 bits wide, 32 rows tall).
    for (u8 cy = 0; cy < FRAMEBUFFER_FONT_HEIGHT; cy++) {
        // Each row is 2 bytes: high byte (left 8 pixels), low byte (right 8 pixels)
        u8 row_hi = glyph[cy * 2];
        u8 row_lo = glyph[cy * 2 + 1];

        // Draw left 8 pixels
        for (u8 cx = 0; cx < 8; cx++) {
            u32 color = (row_hi & (0x80 >> cx)) ? fg_color : bg_color;
            framebuffer_put_pixel(state, px + cx, py + cy, color);
        }
        // Draw right 8 pixels
        for (u8 cx = 0; cx < 8; cx++) {
            u32 color = (row_lo & (0x80 >> cx)) ? fg_color : bg_color;
            framebuffer_put_pixel(state, px + 8 + cx, py + cy, color);
        }
    }
}
