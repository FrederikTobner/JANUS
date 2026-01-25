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
 * @file vga_text.h
 * @brief VGA text mode output driver
 */

#ifndef DRIVER_VGA_TEXT_H
#define DRIVER_VGA_TEXT_H

#include <janus/types.h>

#define VGA_TEXT_WIDTH  80
#define VGA_TEXT_HEIGHT 25

/**
 * @brief Initialize VGA text mode output
 */
void vga_text_init();

/**
 * @brief Clear the screen using the current color
 */
void vga_text_clear();

/**
 * @brief Set the VGA text color
 * @param foreground Foreground color (0-15)
 * @param background Background color (0-15)
 */
void vga_text_set_color(u8 foreground, u8 background);

/**
 * @brief Write a character to the VGA text buffer
 * @param c Character to write
 */
void vga_text_putc(char c);

/**
 * @brief Write a null-terminated string to the VGA text buffer
 * @param char_buffer Pointer to the null-terminated character buffer
 *
 * @warning This function expects the character buffer to be null-terminated. Failing to do so will result in UNDEFINED
 * BEHAVIOR
 */
void vga_text_write_string(char const * char_buffer);

#endif
