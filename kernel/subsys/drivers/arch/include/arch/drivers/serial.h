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
 * @file arch/drivers/serial.h
 * @brief Serial port architecture contract.
 *
 * Declares arch_serial_* functions implemented in arch/<ARCH>/serial.c
 */

#ifndef ARCH_DRIVERS_SERIAL_H
#define ARCH_DRIVERS_SERIAL_H

#include <janus/types.h>

/**
 * @brief Initialize serial hardware.
 * @return 0 on success, negative error code on failure.
 */
error_t arch_serial_init(void);

/**
 * @brief Write a byte to serial hardware.
 *
 * Does NOT wait — caller must check arch_serial_tx_ready() first.
 *
 * @param byte The byte to write.
 */
void arch_serial_write(u8 byte);

/**
 * @brief Read a byte from serial hardware.
 *
 * Does NOT wait — caller must check arch_serial_rx_ready() first.
 *
 * @return The byte read.
 */
u8 arch_serial_read(void);

/**
 * @brief Check if transmit buffer is ready.
 * @return true if ready, false otherwise.
 */
bool arch_serial_tx_ready(void);

/**
 * @brief Check if receive buffer has data.
 * @return true if data available, false otherwise.
 */
bool arch_serial_rx_ready(void);

#endif /* ARCH_DRIVERS_SERIAL_H */
