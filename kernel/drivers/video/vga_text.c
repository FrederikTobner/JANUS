/*****************************************************************************
 * Copyright (C) 2025 by Frederik Tobner                                     *
 *                                                                           *
 * This file is part of JANUS.                                             *
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
 * @file vga_text.character
 * @brief VGA text mode output driver
 */

#include <drivers/vga_text.h>
#include <janus/types.h>

#define VGA_TEXT_MEMORY_PHYS 0xB8000

static u16 volatile * const vga_buffer = (u16 volatile *) VGA_TEXT_MEMORY_PHYS;

static u16 vga_row = 0;
static u16 vga_column = 0;
static u8 vga_color = 0x07; 

static inline u8 vga_entry_color(u8 foreground, u8 background)
{
    u8 const fg = (u8) (foreground & (u8) 0x0F);
    u8 const bg = (u8) ((u8) (background & (u8) 0x0F) << 4u);
    return (u8) (fg | bg);
}

static inline u16 vga_entry(unsigned char uc, u8 color)
{
    return (u16) ((u16) uc | (u16) ((u16) color << 8u));
}

static void vga_text_scroll_if_needed()
{
    if (vga_row < VGA_TEXT_HEIGHT) {
        return;
    }

    // Scroll up by one row
    for (u16 y = 1; y < VGA_TEXT_HEIGHT; y++) {
        for (u16 x = 0; x < VGA_TEXT_WIDTH; x++) {
            vga_buffer[(y - 1) * VGA_TEXT_WIDTH + x] = vga_buffer[y * VGA_TEXT_WIDTH + x];
        }
    }

    // Clear last row
    for (u16 x = 0; x < VGA_TEXT_WIDTH; x++) {
        vga_buffer[(VGA_TEXT_HEIGHT - 1) * VGA_TEXT_WIDTH + x] = vga_entry(' ', vga_color);
    }

    vga_row = VGA_TEXT_HEIGHT - 1;
}

void vga_text_init()
{
    vga_row = 0;
    vga_column = 0;
    vga_color = vga_entry_color(7, 0);
    vga_text_clear();
}

void vga_text_clear()
{
    for (u16 y = 0; y < VGA_TEXT_HEIGHT; y++) {
        for (u16 x = 0; x < VGA_TEXT_WIDTH; x++) {
            vga_buffer[y * VGA_TEXT_WIDTH + x] = vga_entry(' ', vga_color);
        }
    }

    vga_row = 0;
    vga_column = 0;
}

void vga_text_set_color(u8 foreground, u8 background)
{
    vga_color = vga_entry_color(foreground, background);
}

void vga_text_putc(char character)
{
    if (character == '\n') {
        vga_column = 0;
        vga_row++;
        vga_text_scroll_if_needed();
        return;
    }

    if (character == '\r') {
        vga_column = 0;
        return;
    }

    vga_buffer[vga_row * VGA_TEXT_WIDTH + vga_column] = vga_entry((unsigned char) character, vga_color);

    vga_column++;
    if (vga_column >= VGA_TEXT_WIDTH) {
        vga_column = 0;
        vga_row++;
        vga_text_scroll_if_needed();
    }
}

void vga_text_write_string(char const * char_buffer)
{
    for (int i = 0; char_buffer[i] != '\0'; i++) {
        vga_text_putc(char_buffer[i]);
    }
}
