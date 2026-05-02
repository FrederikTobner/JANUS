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
/// @brief AArch64 TTY implementation (framebuffer only).
///
/// Implements arch_tty_* contract using framebuffer text rendering.
/// Unlike x86_64, AArch64 doesn't have VGA text mode - only framebuffer.
/// Uses the shared framebuffer implementation with Terminus 16x32 font.

#include <arch/drivers/tty.h>
#include <arch/shared/drivers/framebuffer.h>
#include <janus/attributes.h>
#include <janus/errno.h>

// Framebuffer state - shared implementation
static framebuffer_state_t g_framebuffer_state;

error_t arch_tty_init(display_info_t const * config)
{
    if (config->mode != DISPLAY_MODE_FRAMEBUFFER) {
        // AArch64 requires framebuffer - no fallback
        return JANUS_ENODEV;
    }

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
}

void arch_tty_get_size(u16 * width, u16 * height)
{
    if (width) {
        *width = g_framebuffer_state.text_width;
    }
    if (height) {
        *height = g_framebuffer_state.text_height;
    }
}

void arch_tty_write_cell(u16 x, u16 y, char c, u8 foreground, u8 background)
{
    framebuffer_draw_char(&g_framebuffer_state, x, y, c, foreground, background);
}

void arch_tty_read_cell(__unused u16 x, __unused u16 y, char * c, u8 * foreground, u8 * background)
{
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
}

void arch_tty_set_cursor(__unused u16 x, __unused u16 y)
{
    // Software cursor - for now just track position
    // Could implement visible cursor by inverting colors at cursor position
}
