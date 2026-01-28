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
 * @file uart.c
 * @brief UART Driver (only COM1 with a fixed baud rate of 38400 for now)
 */
#include <arch/io.h>
#include <drivers/uart.h>
#include <janus/types.h>

// Port offsets for COM1
#define UART_DATA_PORT(base)          (base)
#define UART_FIFO_COMMAND_PORT(base)  (base + 2)
#define UART_LINE_COMMAND_PORT(base)  (base + 3)
#define UART_MODEM_COMMAND_PORT(base) (base + 4)
#define UART_LINE_STATUS_PORT(base)   (base + 5)

error_t uart_init(void)
{
    // Disable all interrupts
    outb(UART_COM1 + 1, 0x00);

    // Enable DLAB (set baud rate divisor)
    outb(UART_LINE_COMMAND_PORT(UART_COM1), 0x80);

    // Set divisor to 3 (lo byte) 38400 baud
    outb(UART_DATA_PORT(UART_COM1), 0x03);
    outb(UART_COM1 + 1, 0x00); // (hi byte)

    // 8 bits, no parity, one stop bit
    outb(UART_LINE_COMMAND_PORT(UART_COM1), 0x03);

    // Enable FIFO, clear them, with 14-byte threshold
    outb(UART_FIFO_COMMAND_PORT(UART_COM1), 0xC7);

    // IRQs enabled, RTS/DSR set
    outb(UART_MODEM_COMMAND_PORT(UART_COM1), 0x0B);

    // Set in loopback mode, test the serial chip
    outb(UART_MODEM_COMMAND_PORT(UART_COM1), 0x1E);

    // Test serial chip (send byte 0xAE and check if serial returns same byte)
    outb(UART_DATA_PORT(UART_COM1), 0xAE);
    // Check if serial is faulty (i.e., not same byte as sent)
    if (inb(UART_DATA_PORT(UART_COM1)) != 0xAE) {
        return -1;
    }

    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    outb(UART_MODEM_COMMAND_PORT(UART_COM1), 0x0F);

    return 0;
}

int uart_is_transmit_empty(void)
{
    return inb(UART_LINE_STATUS_PORT(UART_COM1)) & 0x20;
}

void uart_write_char(char c)
{
    // Wait for transmit buffer to be empty
    while (!uart_is_transmit_empty()) {
        ;
    }

    // Send character
    outb(UART_DATA_PORT(UART_COM1), (unsigned char) c);
}

void uart_write_string(char const * str)
{
    for (int i = 0; str[i] != '\0'; i++) {
        uart_write_char(str[i]);
    }
}
