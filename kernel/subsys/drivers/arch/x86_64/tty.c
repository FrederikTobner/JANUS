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

/// @file tty.c
/// @brief x86_64 TTY implementation.
///
/// Implements arch_tty_* contract using either:
/// - VGA text mode (0xB8000) for Multiboot2/GRUB
/// - Framebuffer text rendering for Limine
///
/// The actual VGA and framebuffer logic is delegated to separate modules
/// for clarity and maintainability.

#include <arch/internal/drivers/vga.h>
#include <arch/shared/drivers/framebuffer.h>
#include <drivers/tty.h>
#include <janus/errno.h>

static display_mode_t g_display_mode = DISPLAY_MODE_NONE;

// VGA state
static u16 volatile * g_vga_buffer = NULL;

// Framebuffer state
static framebuffer_state_t g_framebuffer_state;

error_t arch_tty_init(display_info_t const * config)
{
    g_display_mode = config->mode;

    switch (g_display_mode) {
    case DISPLAY_MODE_VGA_TEXT:
        // VGA text mode - only works with identity-mapped memory
        g_vga_buffer = (u16 volatile *) VGA_BUFFER_PHYS;
        return JANUS_OK;

    case DISPLAY_MODE_FRAMEBUFFER:
        framebuffer_init(&g_framebuffer_state,
                         config->framebuffer,
                         config->width,
                         config->height,
                         config->pitch,
                         config->bpp,
                         config->red_mask_shift,
                         config->green_mask_shift,
                         config->blue_mask_shift);
        return JANUS_OK;

    default:
        return JANUS_ENODEV;
    }
}

void arch_tty_get_size(u16 * width, u16 * height)
{
    switch (g_display_mode) {
    case DISPLAY_MODE_VGA_TEXT:
        if (width) {
            *width = VGA_WIDTH;
        }
        if (height) {
            *height = VGA_HEIGHT;
        }
        break;

    case DISPLAY_MODE_FRAMEBUFFER:
        if (width) {
            *width = g_framebuffer_state.text_width;
        }
        if (height) {
            *height = g_framebuffer_state.text_height;
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

void arch_tty_write_cell(u16 x, u16 y, char c, u8 foreground, u8 background)
{
    switch (g_display_mode) {
    case DISPLAY_MODE_VGA_TEXT:
        vga_write_cell(g_vga_buffer, x, y, c, foreground, background);
        break;

    case DISPLAY_MODE_FRAMEBUFFER:
        framebuffer_draw_char(&g_framebuffer_state, x, y, c, foreground, background);
        break;

    default:
        break;
    }
}

void arch_tty_read_cell(u16 x, u16 y, char * c, u8 * foreground, u8 * background)
{
    switch (g_display_mode) {
    case DISPLAY_MODE_VGA_TEXT:
        vga_read_cell(g_vga_buffer, x, y, c, foreground, background);
        break;

    case DISPLAY_MODE_FRAMEBUFFER:
        // Framebuffer doesn't store character data - return defaults
        if (c) {
            *c = ' ';
        }
        if (foreground) {
            *foreground = TTY_COLOR_WHITE;
        }
        if (background) {
            *background = TTY_COLOR_BLACK;
        }
        break;

    default:
        if (c) {
            *c = ' ';
        }
        if (foreground) {
            *foreground = 0;
        }
        if (background) {
            *background = 0;
        }
        break;
    }
}

void arch_tty_set_cursor(u16 x, u16 y)
{
    // Hardware cursor only available in VGA mode
    if (g_display_mode == DISPLAY_MODE_VGA_TEXT) {
        vga_set_cursor(x, y);
    }
    // Framebuffer mode: no hardware cursor (could draw a software cursor)
}
