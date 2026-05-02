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
/// @brief Serial port driver — shared logic.
///
/// Implements the out-of-line serial API declared in <drivers/serial.h>.
/// Architecture-specific hardware access is delegated to arch_serial_*.

#include <arch/drivers/serial.h>
#include <drivers/serial.h>
#include <janus/attributes.h>

__cold error_t drivers_serial_init(u64 hhdm_offset, phys_addr_t kernel_phys_base, virt_addr_t kernel_virt_base)
{
    return arch_serial_init(hhdm_offset, kernel_phys_base, kernel_virt_base);
}

void drivers_serial_putc(char c)
{
    if (c == '\n') {
        while (!arch_serial_tx_ready()) {
            // Wait for transmit buffer to be ready
        }
        arch_serial_write('\r');
    }
    while (!arch_serial_tx_ready()) {
        // Wait for transmit buffer to be ready
    }
    arch_serial_write((u8) c);
}

void drivers_serial_puts(char const * str)
{
    while (*str) {
        drivers_serial_putc(*str++);
    }
}

s32 drivers_serial_getc(void)
{
    if (!arch_serial_rx_ready()) {
        return -1;
    }
    return (s32) arch_serial_read();
}
