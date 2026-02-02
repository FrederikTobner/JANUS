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

#ifndef ARCH_IMPL_DRIVERS_FRAMEBUFFER_H
#define ARCH_IMPL_DRIVERS_FRAMEBUFFER_H

/**
 * @file framebuffer.h
 * @brief Framebuffer text rendering interface.
 *
 * Provides text rendering on a linear framebuffer using the Terminus 16x32 bitmap font.
 * Used for Limine boot protocol where VGA text mode isn't available.
 */

#include <janus/types.h>

#include <arch/internal/drivers/terminus.h>

#define FRAMEBUFFER_FONT_WIDTH  TERMINUS_WIDTH
#define FRAMEBUFFER_FONT_HEIGHT TERMINUS_HEIGHT

/**
 * @brief Framebuffer state for text rendering.
 */
typedef struct {
    u8 volatile * base; /**< Framebuffer base address */
    u64 pitch;          /**< Bytes per row */
    u64 width;          /**< Width in pixels */
    u64 height;         /**< Height in pixels */
    u16 bpp;            /**< Bits per pixel (24 or 32) */
    u8 red_shift;       /**< Red channel bit position */
    u8 green_shift;     /**< Green channel bit position */
    u8 blue_shift;      /**< Blue channel bit position */
    u16 text_width;     /**< Width in characters */
    u16 text_height;    /**< Height in characters */
} framebuffer_state_t;

/**
 * @brief Standard 16-color palette (VGA compatible).
 *
 * Maps TTY color indices (0-15) to RGB values.
 */
static u32 const framebuffer_color_palette[16] = {
    0x000000, /* Black */
    0x0000AA, /* Blue */
    0x00AA00, /* Green */
    0x00AAAA, /* Cyan */
    0xAA0000, /* Red */
    0xAA00AA, /* Magenta */
    0xAA5500, /* Brown */
    0xAAAAAA, /* Light Grey */
    0x555555, /* Dark Grey */
    0x5555FF, /* Light Blue */
    0x55FF55, /* Light Green */
    0x55FFFF, /* Light Cyan */
    0xFF5555, /* Light Red */
    0xFF55FF, /* Light Magenta */
    0xFFFF55, /* Yellow */
    0xFFFFFF, /* White */
};

/**
 * @brief Initialize framebuffer state from configuration.
 *
 * @param state    Framebuffer state to initialize
 * @param base     Framebuffer base address
 * @param width    Width in pixels
 * @param height   Height in pixels
 * @param pitch    Bytes per row
 * @param bpp      Bits per pixel
 * @param r_shift  Red channel bit position
 * @param g_shift  Green channel bit position
 * @param b_shift  Blue channel bit position
 */
static inline void framebuffer_init(framebuffer_state_t * state,
                                    void * base,
                                    u64 width,
                                    u64 height,
                                    u64 pitch,
                                    u16 bpp,
                                    u8 r_shift,
                                    u8 g_shift,
                                    u8 b_shift)
{
    state->base = (u8 volatile *) base;
    state->width = width;
    state->height = height;
    state->pitch = pitch;
    state->bpp = bpp;
    state->red_shift = r_shift;
    state->green_shift = g_shift;
    state->blue_shift = b_shift;
    state->text_width = (u16) (width / FRAMEBUFFER_FONT_WIDTH);
    state->text_height = (u16) (height / FRAMEBUFFER_FONT_HEIGHT);
}

/**
 * @brief Put a single pixel on the framebuffer.
 *
 * @param state Framebuffer state
 * @param x     X coordinate (pixels)
 * @param y     Y coordinate (pixels)
 * @param color RGB color (0x00RRGGBB)
 */
static inline void framebuffer_put_pixel(framebuffer_state_t const * state, u64 x, u64 y, u32 color)
{
    if (x >= state->width || y >= state->height) {
        return;
    }

    u64 offset = y * state->pitch + x * (state->bpp / 8);
    u32 pixel = ((color >> 16) & 0xFF) << state->red_shift | ((color >> 8) & 0xFF) << state->green_shift |
                (color & 0xFF) << state->blue_shift;

    if (state->bpp == 32) {
        *((u32 volatile *) (state->base + offset)) = pixel;
    } else if (state->bpp == 24) {
        state->base[offset] = pixel & 0xFF;
        state->base[offset + 1] = (pixel >> 8) & 0xFF;
        state->base[offset + 2] = (pixel >> 16) & 0xFF;
    }
}

/**
 * @brief Draw a character at the specified text position.
 *
 * @param state Framebuffer state
 * @param column   Column (character position)
 * @param row   Row (character position)
 * @param c     Character to draw
 * @param foreground    Foreground color index (0-15)
 * @param background    Background color index (0-15)
 */
void framebuffer_draw_char(
    framebuffer_state_t const * state, u16 column, u16 row, char c, u8 foreground, u8 background);

#endif /* ARCH_IMPL_DRIVERS_FRAMEBUFFER_H */
