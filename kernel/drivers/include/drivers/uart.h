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
 * @file uart.h
 * @brief UART Driver (only COM1 with a fixed baud rate of 38400 for now)
 */

#ifndef DRIVER_UART_H
#define DRIVER_UART_H
#include "janus/types.h"
#define UART_COM1 0x3F8

// TODO: Add support for COM2, COM3, COM4 and make the baud rate configurable

/**
 * @brief Initialize UART COM1
 * @return 0 on success, non-zero on failure
 */
error_t uart_init();

/**
 * @brief Check if the UART transmit buffer is empty
 * @return 1 if empty, 0 otherwise
 */
int uart_is_transmit_empty();

/**
 * @brief Write a character to the universal asynchronous receiver/transmitter (UART)
 * @param character Character to write
 */
void uart_write_char(char character);

/**
 * @brief Write a null-terminated string to the universal asynchronous receiver/transmitter (UART)
 * @param char_buffer Pointer to the null-terminated character buffer
 *
 * @warning This function expects the character buffer to be null-terminated. Failing to do so will result in UNDEFINED
 * BEHAVIOR
 */
void uart_write_string(char const * char_buffer);

#endif
