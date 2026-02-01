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
 * @brief Framebuffer text rendering implementation.
 *
 * Contains the more complex character drawing logic that benefits
 * from being in a separate compilation unit.
 */

#include <arch/internal/drivers/font8x16.h>
#include <arch/internal/drivers/framebuffer.h>

void framebuffer_draw_char(framebuffer_state_t const * state, u16 column, u16 row, char c, u8 foreground, u8 background)
{
    if (!state->base || column >= state->text_width || row >= state->text_height) {
        return;
    }

    u64 px = column * FRAMEBUFFER_FONT_WIDTH;
    u64 py = row * FRAMEBUFFER_FONT_HEIGHT;
    u32 fg_color = framebuffer_color_palette[foreground & 0x0F];
    u32 bg_color = framebuffer_color_palette[background & 0x0F];

    /* Get font bitmap for this character */
    u8 const * glyph;
    if (c >= 32 && c < 127) {
        glyph = font8x16_data + (c - 32) * FRAMEBUFFER_FONT_HEIGHT;
    } else {
        /* Use space for unprintable characters */
        glyph = font8x16_data;
    }

    /* Draw each pixel of the character */
    for (u8 cy = 0; cy < FRAMEBUFFER_FONT_HEIGHT; cy++) {
        u8 row_data = glyph[cy];
        for (u8 cx = 0; cx < FRAMEBUFFER_FONT_WIDTH; cx++) {
            u32 color = (row_data & (0x80 >> cx)) ? fg_color : bg_color;
            framebuffer_put_pixel(state, px + cx, py + cy, color);
        }
    }
}
