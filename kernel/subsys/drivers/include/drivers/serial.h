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
 * @file serial.h
 * @brief Serial port driver interface.
 *
 * This header contains:
 * - Public API (serial_*)
 * - Shared inline logic
 *
 * Architecture contract (arch_serial_*) is in <arch/drivers/serial.h>.
 */

#ifndef DRIVERS_SERIAL_H
#define DRIVERS_SERIAL_H

#include <arch/drivers/serial.h>
#include <janus/attributes.h>
#include <janus/types.h>

/*===========================================================================
 * Public API — Inline implementations using arch primitives
 *===========================================================================*/

/**
 * @brief Initialize the serial port.
 * @return 0 on success, negative error code on failure.
 */
static __always_inline error_t serial_init(void)
{
    return arch_serial_init();
}

/**
 * @brief Write a single character (blocking).
 * @param c The character to write.
 */
static __always_inline void serial_putc(char c)
{
    while (!arch_serial_tx_ready()) {
        /* Wait for transmit buffer to be ready */
    }
    arch_serial_write((u8) c);
}

/**
 * @brief Write a null-terminated string with CR+LF conversion.
 * @param str The string to write.
 */
static __always_inline void serial_puts(char const * str)
{
    while (*str) {
        if (*str == '\n') {
            serial_putc('\r'); /* CR+LF for serial terminals */
        }
        serial_putc(*str++);
    }
}

/**
 * @brief Check if transmit buffer is ready.
 * @return true if ready, false otherwise.
 */
static __always_inline bool serial_tx_ready(void)
{
    return arch_serial_tx_ready();
}

/**
 * @brief Read a character (non-blocking).
 * @return The character read, or -1 if none available.
 */
static __always_inline s32 serial_getc(void)
{
    if (!arch_serial_rx_ready()) {
        return -1;
    }
    return (s32) arch_serial_read();
}

/**
 * @brief Check if receive buffer has data.
 * @return true if data available, false otherwise.
 */
static __always_inline bool serial_rx_ready(void)
{
    return arch_serial_rx_ready();
}

#endif /* DRIVERS_SERIAL_H */
