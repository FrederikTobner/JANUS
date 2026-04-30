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

#ifndef ARCH_IMPL_DRIVERS_VGA_H
#define ARCH_IMPL_DRIVERS_VGA_H

/// @file vga.h
/// @brief x86_64 VGA text mode driver.
///
/// Provides low-level VGA text mode (0xB8000) operations.
/// Simple functions are inlined for performance.

#include <arch/impl/drivers/io.h>
#include <janus/attributes.h>
#include <janus/types.h>

#define VGA_WIDTH       80
#define VGA_HEIGHT      25
#define VGA_BUFFER_PHYS 0xB8000
#define VGA_CRTC_INDEX  0x3D4
#define VGA_CRTC_DATA   0x3D5

/// @brief Create a VGA text mode character entry.
///
/// @param character  Character to display
/// @param foreground Foreground color (0-15)
/// @param background Background color (0-15)
/// @return Combined character/attribute word for VGA buffer
static __always_inline u16 vga_entry(char character, u8 foreground, u8 background)
{
    u8 color = (u8) ((foreground & 0x0F) | ((background & 0x0F) << 4));
    return (u16) ((u16) (u8) character | (u16) ((u16) color << 8));
}

/// @brief Write a character to the VGA buffer.
///
/// @param buffer Pointer to VGA buffer
/// @param x      Column (0 to VGA_WIDTH-1)
/// @param y      Row (0 to VGA_HEIGHT-1)
/// @param character      Character to write
/// @param foreground     Foreground color
/// @param background     Background color
static __always_inline void
vga_write_cell(u16 volatile * buffer, u16 x, u16 y, char character, u8 foreground, u8 background)
{
    if (buffer && x < VGA_WIDTH && y < VGA_HEIGHT) {
        buffer[y * VGA_WIDTH + x] = vga_entry(character, foreground, background);
    }
}

/// @brief Read a character from the VGA buffer.
///
/// @param buffer Pointer to VGA buffer
/// @param x      Column (0 to VGA_WIDTH-1)
/// @param y      Row (0 to VGA_HEIGHT-1)
/// @param character      Output: character (may be NULL)
/// @param foreground     Output: foreground color (may be NULL)
/// @param background     Output: background color (may be NULL)
static __always_inline void
vga_read_cell(u16 volatile * buffer, u16 x, u16 y, char * character, u8 * foreground, u8 * background)
{
    if (buffer && x < VGA_WIDTH && y < VGA_HEIGHT) {
        u16 entry = buffer[y * VGA_WIDTH + x];
        if (character) {
            *character = (char) (entry & 0xFF);
        }
        if (foreground) {
            *foreground = (entry >> 8) & 0x0F;
        }
        if (background) {
            *background = (entry >> 12) & 0x0F;
        }
    }
}

/// @brief Set the VGA hardware cursor position.
///
/// @param x Column (0 to VGA_WIDTH-1)
/// @param y Row (0 to VGA_HEIGHT-1)
static __always_inline void vga_set_cursor(u16 x, u16 y)
{
    u16 pos = (u16) (y * VGA_WIDTH + x);
    outb(VGA_CRTC_INDEX, 0x0F);
    outb(VGA_CRTC_DATA, (u8) (pos & 0xFF));
    outb(VGA_CRTC_INDEX, 0x0E);
    outb(VGA_CRTC_DATA, (u8) ((pos >> 8) & 0xFF));
}

#endif /* ARCH_IMPL_DRIVERS_VGA_H */
