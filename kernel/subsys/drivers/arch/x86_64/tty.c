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
 * @file tty.c
 * @brief x86_64 TTY implementation (VGA text mode + framebuffer).
 *
 * Implements arch_tty_* contract from <arch/drivers/tty.h>.
 * Supports two backends:
 * - VGA text mode (0xB8000) for Multiboot2/GRUB
 * - Framebuffer text rendering for Limine
 */

#include <arch/drivers/tty.h>
#include <arch/impl/drivers/io.h>

/*===========================================================================
 * Display mode selection
 *===========================================================================*/

typedef enum {
    DISPLAY_MODE_NONE,
    DISPLAY_MODE_VGA,
    DISPLAY_MODE_FRAMEBUFFER,
} display_mode_t;

static display_mode_t g_display_mode = DISPLAY_MODE_NONE;

/*===========================================================================
 * VGA Text Mode Backend
 *===========================================================================*/

#define VGA_WIDTH       80
#define VGA_HEIGHT      25
#define VGA_BUFFER_PHYS 0xB8000
#define VGA_CRTC_INDEX  0x3D4
#define VGA_CRTC_DATA   0x3D5

static volatile u16 * vga_buffer = NULL;

static inline u16 vga_entry(char c, u8 fg, u8 bg)
{
    u8 color = (u8) ((fg & 0x0F) | ((bg & 0x0F) << 4));
    return (u16) ((u16) (u8) c | (u16) ((u16) color << 8));
}

/*===========================================================================
 * Framebuffer Text Rendering Backend
 *===========================================================================*/

/* Simple 8x16 bitmap font (embedded below) */
#define FONT_WIDTH  8
#define FONT_HEIGHT 16

static volatile u8 * fb_base = NULL;
static u64 fb_pitch = 0;
static u64 fb_width = 0;
static u64 fb_height = 0;
static u16 fb_bpp = 0;
static u8 fb_red_shift = 0;
static u8 fb_green_shift = 0;
static u8 fb_blue_shift = 0;

/* Text dimensions in characters */
static u16 fb_text_width = 0;
static u16 fb_text_height = 0;

/* Color palette: map TTY colors (0-15) to RGB values */
static u32 const color_palette[16] = {
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

/* Basic 8x16 font data - ASCII 32-126 */
/* Each character is 16 bytes (16 rows of 8 pixels) */
#include "font8x16.h"

static inline void fb_put_pixel(u64 x, u64 y, u32 color)
{
    if (x >= fb_width || y >= fb_height) {
        return;
    }
    u64 offset = y * fb_pitch + x * (fb_bpp / 8);
    u32 pixel = ((color >> 16) & 0xFF) << fb_red_shift | ((color >> 8) & 0xFF) << fb_green_shift |
                (color & 0xFF) << fb_blue_shift;

    if (fb_bpp == 32) {
        *((u32 volatile *) (fb_base + offset)) = pixel;
    } else if (fb_bpp == 24) {
        fb_base[offset] = pixel & 0xFF;
        fb_base[offset + 1] = (pixel >> 8) & 0xFF;
        fb_base[offset + 2] = (pixel >> 16) & 0xFF;
    }
}

static void fb_draw_char(u16 col, u16 row, char c, u8 fg, u8 bg)
{
    u64 px = col * FONT_WIDTH;
    u64 py = row * FONT_HEIGHT;
    u32 fg_color = color_palette[fg & 0x0F];
    u32 bg_color = color_palette[bg & 0x0F];

    /* Get font bitmap for this character */
    u8 const * glyph;
    if (c >= 32 && c < 127) {
        glyph = font8x16_data + (c - 32) * FONT_HEIGHT;
    } else {
        /* Use space for unprintable characters */
        glyph = font8x16_data;
    }

    /* Draw each pixel of the character */
    for (u8 cy = 0; cy < FONT_HEIGHT; cy++) {
        u8 row_data = glyph[cy];
        for (u8 cx = 0; cx < FONT_WIDTH; cx++) {
            u32 color = (row_data & (0x80 >> cx)) ? fg_color : bg_color;
            fb_put_pixel(px + cx, py + cy, color);
        }
    }
}

static void fb_read_char(u16 col, u16 row, char * c, u8 * fg, u8 * bg)
{
    /* Framebuffer doesn't store character data - return defaults */
    (void) col;
    (void) row;
    if (c) {
        *c = ' ';
    }
    if (fg) {
        *fg = TTY_COLOR_WHITE;
    }
    if (bg) {
        *bg = TTY_COLOR_BLACK;
    }
}

/*===========================================================================
 * Arch TTY Interface Implementation
 *===========================================================================*/

error_t arch_tty_init(tty_display_config_t const * config)
{
    if (config == NULL || config->framebuffer == NULL) {
        /* VGA text mode - only works with identity-mapped memory */
        vga_buffer = (u16 volatile *) VGA_BUFFER_PHYS;
        g_display_mode = DISPLAY_MODE_VGA;
        return 0;
    }

    /* Framebuffer mode */
    fb_base = (u8 volatile *) config->framebuffer;
    fb_width = config->width;
    fb_height = config->height;
    fb_pitch = config->pitch;
    fb_bpp = config->bpp;
    fb_red_shift = config->red_mask_shift;
    fb_green_shift = config->green_mask_shift;
    fb_blue_shift = config->blue_mask_shift;

    /* Calculate text dimensions */
    fb_text_width = (u16) (fb_width / FONT_WIDTH);
    fb_text_height = (u16) (fb_height / FONT_HEIGHT);

    g_display_mode = DISPLAY_MODE_FRAMEBUFFER;
    return 0;
}

void arch_tty_get_size(u16 * width, u16 * height)
{
    switch (g_display_mode) {
    case DISPLAY_MODE_VGA:
        if (width) {
            *width = VGA_WIDTH;
        }
        if (height) {
            *height = VGA_HEIGHT;
        }
        break;

    case DISPLAY_MODE_FRAMEBUFFER:
        if (width) {
            *width = fb_text_width;
        }
        if (height) {
            *height = fb_text_height;
        }
        break;

    default:
        if (width) {
            *width = 0;
        }
        if (height) {
            *height = 0;
        }
        break;
    }
}

void arch_tty_write_cell(u16 x, u16 y, char c, u8 fg, u8 bg)
{
    switch (g_display_mode) {
    case DISPLAY_MODE_VGA:
        if (vga_buffer && x < VGA_WIDTH && y < VGA_HEIGHT) {
            vga_buffer[y * VGA_WIDTH + x] = vga_entry(c, fg, bg);
        }
        break;

    case DISPLAY_MODE_FRAMEBUFFER:
        if (fb_base && x < fb_text_width && y < fb_text_height) {
            fb_draw_char(x, y, c, fg, bg);
        }
        break;

    default:
        break;
    }
}

void arch_tty_read_cell(u16 x, u16 y, char * c, u8 * fg, u8 * bg)
{
    switch (g_display_mode) {
    case DISPLAY_MODE_VGA:
        if (vga_buffer && x < VGA_WIDTH && y < VGA_HEIGHT) {
            u16 entry = vga_buffer[y * VGA_WIDTH + x];
            if (c) {
                *c = (char) (entry & 0xFF);
            }
            if (fg) {
                *fg = (entry >> 8) & 0x0F;
            }
            if (bg) {
                *bg = (entry >> 12) & 0x0F;
            }
        }
        break;

    case DISPLAY_MODE_FRAMEBUFFER:
        fb_read_char(x, y, c, fg, bg);
        break;

    default:
        if (c) {
            *c = ' ';
        }
        if (fg) {
            *fg = 0;
        }
        if (bg) {
            *bg = 0;
        }
        break;
    }
}

void arch_tty_set_cursor(u16 x, u16 y)
{
    /* Hardware cursor only available in VGA mode */
    if (g_display_mode == DISPLAY_MODE_VGA) {
        u16 pos = (u16) (y * VGA_WIDTH + x);
        outb(VGA_CRTC_INDEX, 0x0F);
        outb(VGA_CRTC_DATA, (u8) (pos & 0xFF));
        outb(VGA_CRTC_INDEX, 0x0E);
        outb(VGA_CRTC_DATA, (u8) ((pos >> 8) & 0xFF));
    }
    /* Framebuffer mode: no hardware cursor (could draw a software cursor) */
}
