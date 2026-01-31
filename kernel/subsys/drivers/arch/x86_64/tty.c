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
 * @brief x86_64 TTY implementation (VGA text mode).
 *
 * Implements arch_tty_* contract from <arch/drivers/tty.h>.
 */

#include <arch/drivers/tty.h>
#include <arch/impl/drivers/io.h>

/* VGA text mode constants */
#define VGA_WIDTH      80
#define VGA_HEIGHT     25
#define VGA_BUFFER     ((volatile u16 *) 0xB8000)
#define VGA_CRTC_INDEX 0x3D4
#define VGA_CRTC_DATA  0x3D5

static inline u16 vga_entry(char c, u8 fg, u8 bg)
{
    u8 color = (u8) ((fg & 0x0F) | ((bg & 0x0F) << 4));
    return (u16) ((u16) (u8) c | (u16) ((u16) color << 8));
}

error_t arch_tty_init(void)
{
    /* VGA text mode is already set up by BIOS/bootloader */
    return 0;
}

void arch_tty_get_size(u16 * width, u16 * height)
{
    if (width) {
        *width = VGA_WIDTH;
    }
    if (height) {
        *height = VGA_HEIGHT;
    }
}

void arch_tty_write_cell(u16 x, u16 y, char c, u8 fg, u8 bg)
{
    if (x < VGA_WIDTH && y < VGA_HEIGHT) {
        VGA_BUFFER[y * VGA_WIDTH + x] = vga_entry(c, fg, bg);
    }
}

void arch_tty_read_cell(u16 x, u16 y, char * c, u8 * fg, u8 * bg)
{
    if (x < VGA_WIDTH && y < VGA_HEIGHT) {
        u16 entry = VGA_BUFFER[y * VGA_WIDTH + x];
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
}

void arch_tty_set_cursor(u16 x, u16 y)
{
    u16 pos = (u16) (y * VGA_WIDTH + x);
    outb(VGA_CRTC_INDEX, 0x0F);
    outb(VGA_CRTC_DATA, (u8) (pos & 0xFF));
    outb(VGA_CRTC_INDEX, 0x0E);
    outb(VGA_CRTC_DATA, (u8) ((pos >> 8) & 0xFF));
}
