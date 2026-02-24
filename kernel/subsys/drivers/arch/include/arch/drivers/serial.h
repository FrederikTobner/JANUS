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
 *
 * @param hhdm_offset Higher Half Direct Map offset for translating physical
 *                    addresses to virtual addresses. Pass 0 for identity-mapped
 *                    boot (Multiboot2) or the HHDM offset for Limine.
 *                    On x86_64 this is ignored (port I/O doesn't need HHDM).
 *                    On AArch64 this is required for MMIO access.
 * @param kernel_phys_base Physical base address of the kernel image.
 *                         On x86_64 this is ignored.
 *                         On AArch64 this is required for page table allocation.
 * @param kernel_virt_base Virtual base address of the kernel image.
 *                         On x86_64 this is ignored.
 *                         On AArch64 this is required for page table allocation.
 * @return 0 on success, negative error code on failure.
 */
error_t arch_serial_init(u64 hhdm_offset, phys_addr_t kernel_phys_base, virt_addr_t kernel_virt_base);

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
