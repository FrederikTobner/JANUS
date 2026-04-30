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

/// @file serial.h
/// @brief Serial port driver interface.
///
/// This header contains:
/// - Public API (serial_*)
/// - Shared inline logic
///
/// Architecture contract (arch_serial_*) is in <arch/drivers/serial.h>.

#ifndef DRIVERS_SERIAL_H
#define DRIVERS_SERIAL_H

#include <arch/drivers/serial.h>
#include <janus/attributes.h>
#include <janus/types.h>

/// @brief Initialize the serial port.
///
/// @param hhdm_offset Higher Half Direct Map offset for physical to virtual
///                    address translation. Pass 0 for identity-mapped boot.
/// @param kernel_phys_base Physical base address of the kernel image.
/// @param kernel_virt_base Virtual base address of the kernel image.
/// @return 0 on success, negative error code on failure.
static __always_inline error_t drivers_serial_init(u64 hhdm_offset,
                                                   phys_addr_t kernel_phys_base,
                                                   virt_addr_t kernel_virt_base)
{
    return arch_serial_init(hhdm_offset, kernel_phys_base, kernel_virt_base);
}

/// @brief Write a single character (blocking) with CR+LF conversion.
///
/// Bare '\n' is automatically preceded by '\r' so that serial
/// terminals receive the expected CR+LF line ending.
///
/// @param c The character to write.
static __always_inline void drivers_serial_putc(char c)
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

/// @brief Write a null-terminated string (blocking).
///
/// CR+LF conversion is handled by drivers_serial_putc.
///
/// @param str The string to write.
static __always_inline void drivers_serial_puts(char const * str)
{
    while (*str) {
        drivers_serial_putc(*str++);
    }
}

/// @brief Check if transmit buffer is ready.
/// @return true if ready, false otherwise.
static __always_inline bool drivers_serial_tx_ready(void)
{
    return arch_serial_tx_ready();
}

/// @brief Read a character (non-blocking).
/// @return The character read, or -1 if none available.
static __always_inline s32 drivers_serial_getc(void)
{
    if (!arch_serial_rx_ready()) {
        return -1;
    }
    return (s32) arch_serial_read();
}

/// @brief Check if receive buffer has data.
/// @return true if data available, false otherwise.
static __always_inline bool drivers_serial_rx_ready(void)
{
    return arch_serial_rx_ready();
}

#endif /* DRIVERS_SERIAL_H */
