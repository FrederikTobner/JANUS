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
/// @brief AArch64 serial implementation (PL011 UART).
///
/// Implements arch_serial_* contract from <arch/drivers/serial.h>.
///
/// On AArch64 with Limine, the HHDM only maps RAM, not device MMIO.
/// We use mmu_map_mmio() to map the PL011 UART with device memory attributes.

#include <arch/drivers/serial.h>
#include <arch/internal/drivers/mmio.h>
#include <janus/errno.h>
#include <janus/types.h>
#include <page_tables/mmu.h>

// PL011 UART physical base address for QEMU virt machine
#define PL011_PHYS_BASE    0x09000000UL

// PL011 register offsets from base
#define PL011_REG_DR       0x00 // Data Register
#define PL011_REG_FR       0x18 // Flag Register
#define PL011_REG_IBRD     0x24 // Integer Baud Rate Divisor
#define PL011_REG_FBRD     0x28 // Fractional Baud Rate Divisor
#define PL011_REG_LCR_H    0x2C // Line Control Register
#define PL011_REG_CR       0x30 // Control Register
#define PL011_REG_IMSC     0x38 // Interrupt Mask Set/Clear

// Flag register bits
#define PL011_FR_TXFF      (1 << 5) // Transmit FIFO full
#define PL011_FR_RXFE      (1 << 4) // Receive FIFO empty
#define PL011_FR_BUSY      (1 << 3) // UART busy

// Control register bits
#define PL011_CR_RXE       (1 << 9) // Receive enable
#define PL011_CR_TXE       (1 << 8) // Transmit enable
#define PL011_CR_UARTEN    (1 << 0) // UART enable

// Line control register bits
#define PL011_LCR_H_WLEN_8 (3 << 5) // 8-bit word length
#define PL011_LCR_H_FEN    (1 << 4) // FIFO enable

// Virtual base address (computed at init time via MMIO mapping)
static u64 g_pl011_base = 0;

/// @brief Get virtual address for a PL011 register
static inline u64 pl011_reg(u32 offset)
{
    return g_pl011_base + offset;
}

error_t arch_serial_init(u64 hhdm_offset, phys_addr_t kernel_phys_base, virt_addr_t kernel_virt_base)
{
    // Initialize the MMU module for MMIO mapping.
    // This must be done before mmu_map_mmio() can be called.
    error_t mmu_err = mmu_init(hhdm_offset, kernel_phys_base, kernel_virt_base);
    if (mmu_err != JANUS_OK) {
        return mmu_err;
    }

    // Map PL011 UART MMIO region into kernel address space.
    // The HHDM only maps RAM, so we need explicit MMIO mapping.
    virt_addr_t mapped = mmu_map_mmio(PL011_PHYS_BASE, 0x1000);
    if (mapped == 0) {
        // Failed to map UART - fall back to framebuffer-only output
        return JANUS_ENODEV;
    }
    g_pl011_base = mapped;

    // Disable UART while configuring
    mmio_write32(pl011_reg(PL011_REG_CR), 0);

    // Clear all pending interrupts
    mmio_write32(pl011_reg(PL011_REG_IMSC), 0);

    // Set baud rate (115200 at 24MHz UART clock for QEMU virt)
    // Divisor = 24000000 / (16 * 115200) = 13.0208...
    // Integer part = 13, Fractional part = 0.0208 * 64 = 1.33 ≈ 1
    mmio_write32(pl011_reg(PL011_REG_IBRD), 13);
    mmio_write32(pl011_reg(PL011_REG_FBRD), 1);

    // 8 bits, no parity, 1 stop bit, FIFO enabled
    mmio_write32(pl011_reg(PL011_REG_LCR_H), PL011_LCR_H_WLEN_8 | PL011_LCR_H_FEN);

    // Enable UART, TX, and RX
    mmio_write32(pl011_reg(PL011_REG_CR), PL011_CR_UARTEN | PL011_CR_TXE | PL011_CR_RXE);

    return JANUS_OK;
}

void arch_serial_write(u8 byte)
{
    mmio_write32(pl011_reg(PL011_REG_DR), byte);
}

u8 arch_serial_read(void)
{
    return (u8) mmio_read32(pl011_reg(PL011_REG_DR));
}

bool arch_serial_tx_ready(void)
{
    // Ready if TX FIFO is not full
    return (mmio_read32(pl011_reg(PL011_REG_FR)) & PL011_FR_TXFF) == 0;
}

bool arch_serial_rx_ready(void)
{
    // Ready if RX FIFO is not empty
    return (mmio_read32(pl011_reg(PL011_REG_FR)) & PL011_FR_RXFE) == 0;
}
