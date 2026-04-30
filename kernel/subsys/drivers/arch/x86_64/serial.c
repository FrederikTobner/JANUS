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

/// @file serial.c
/// @brief x86_64 serial implementation (16550 UART on COM1).
///
/// Implements arch_serial_* contract from <arch/drivers/serial.h>.

#include <arch/drivers/serial.h>
#include <arch/impl/drivers/io.h>

// COM1 port addresses
#define COM1               0x3F8
#define COM1_DATA          (COM1 + 0)
#define COM1_INT_EN        (COM1 + 1)
#define COM1_FIFO_CTRL     (COM1 + 2)
#define COM1_LINE_CTRL     (COM1 + 3)
#define COM1_MODEM_CTRL    (COM1 + 4)
#define COM1_LINE_STAT     (COM1 + 5)

// Line status register bits
#define LINE_STAT_TX_EMPTY 0x20
#define LINE_STAT_RX_READY 0x01

error_t arch_serial_init(__unused u64 hhdm_offset, __unused u64 kernel_phys_base, __unused u64 kernel_virt_base)
{
    // x86_64 uses port I/O, not MMIO, so these parameters are unused

    // Disable all interrupts
    outb(COM1_INT_EN, 0x00);

    // Enable DLAB (set baud rate divisor)
    outb(COM1_LINE_CTRL, 0x80);

    // Set divisor to 3 (lo byte) for 38400 baud
    outb(COM1_DATA, 0x03);
    outb(COM1_INT_EN, 0x00); // (hi byte)

    // 8 bits, no parity, one stop bit (8N1)
    outb(COM1_LINE_CTRL, 0x03);

    // Enable FIFO, clear them, with 14-byte threshold
    outb(COM1_FIFO_CTRL, 0xC7);

    // IRQs enabled, RTS/DSR set
    outb(COM1_MODEM_CTRL, 0x0B);

    // Set in loopback mode to test the serial chip
    outb(COM1_MODEM_CTRL, 0x1E);

    // Test serial chip (send byte 0xAE and check if serial returns same byte)
    outb(COM1_DATA, 0xAE);
    if (inb(COM1_DATA) != 0xAE) {
        return -1;
    }

    // If serial is not faulty, set it in normal operation mode
    outb(COM1_MODEM_CTRL, 0x0F);

    return 0;
}

bool arch_serial_tx_ready(void)
{
    return (inb(COM1_LINE_STAT) & LINE_STAT_TX_EMPTY) != 0;
}

void arch_serial_write(u8 byte)
{
    outb(COM1_DATA, byte);
}

bool arch_serial_rx_ready(void)
{
    return (inb(COM1_LINE_STAT) & LINE_STAT_RX_READY) != 0;
}

u8 arch_serial_read(void)
{
    return inb(COM1_DATA);
}
