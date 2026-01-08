/*****************************************************************************
 * Copyright (C) 2025 by Frederik Tobner                                     *
 *                                                                           *
 * This file is part of TinyOS.                                             *
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
 * @file serial.c
 * @brief Serial Port Driver (only COM1 with a fixed baud rate of 38400 for now)
 */

#include <drivers/serial.h>
#include <tinyos/types.h>

// Port offsets for COM1
#define SERIAL_DATA_PORT(base)          (base)
#define SERIAL_FIFO_COMMAND_PORT(base)  (base + 2)
#define SERIAL_LINE_COMMAND_PORT(base)  (base + 3)
#define SERIAL_MODEM_COMMAND_PORT(base) (base + 4)
#define SERIAL_LINE_STATUS_PORT(base)   (base + 5)

static inline void outb(u16 port, u8 value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline u8 inb(u16 port)
{
    u8 ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

int serial_init()
{
    // Disable all interrupts
    outb(SERIAL_COM1 + 1, 0x00);

    // Enable DLAB (set baud rate divisor)
    outb(SERIAL_LINE_COMMAND_PORT(SERIAL_COM1), 0x80);

    // Set divisor to 3 (lo byte) 38400 baud
    outb(SERIAL_DATA_PORT(SERIAL_COM1), 0x03);
    outb(SERIAL_COM1 + 1, 0x00); // (hi byte)

    // 8 bits, no parity, one stop bit
    outb(SERIAL_LINE_COMMAND_PORT(SERIAL_COM1), 0x03);

    // Enable FIFO, clear them, with 14-byte threshold
    outb(SERIAL_FIFO_COMMAND_PORT(SERIAL_COM1), 0xC7);

    // IRQs enabled, RTS/DSR set
    outb(SERIAL_MODEM_COMMAND_PORT(SERIAL_COM1), 0x0B);

    // Set in loopback mode, test the serial chip
    outb(SERIAL_MODEM_COMMAND_PORT(SERIAL_COM1), 0x1E);

    // Test serial chip (send byte 0xAE and check if serial returns same byte)
    outb(SERIAL_DATA_PORT(SERIAL_COM1), 0xAE);

    // Check if serial is faulty (i.e., not same byte as sent)
    if (inb(SERIAL_DATA_PORT(SERIAL_COM1)) != 0xAE) {
        return 1;
    }

    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    outb(SERIAL_MODEM_COMMAND_PORT(SERIAL_COM1), 0x0F);

    return 0;
}

int serial_is_transmit_empty()
{
    return inb(SERIAL_LINE_STATUS_PORT(SERIAL_COM1)) & 0x20;
}

void serial_write_char(char c)
{
    // Wait for transmit buffer to be empty
    while (!serial_is_transmit_empty())
        ;

    // Send character
    outb(SERIAL_DATA_PORT(SERIAL_COM1), c);
}

void serial_write_string(char const * str)
{
    for (int i = 0; str[i] != '\0'; i++) {
        serial_write_char(str[i]);
    }
}
