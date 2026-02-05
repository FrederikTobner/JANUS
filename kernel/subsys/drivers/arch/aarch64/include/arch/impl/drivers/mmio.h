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
 * @file arch/impl/drivers/mmio.h
 * @brief AArch64 Memory-Mapped I/O helpers.
 *
 * Provides inline functions for MMIO access on AArch64.
 * Unlike x86_64 which uses port I/O (inb/outb), AArch64 uses MMIO exclusively.
 */

#ifndef AARCH64_IMPL_DRIVERS_MMIO_H
#define AARCH64_IMPL_DRIVERS_MMIO_H

#include <janus/attributes.h>
#include <janus/types.h>

/**
 * @brief Write 8-bit value to MMIO address.
 * @param addr MMIO address.
 * @param value Value to write.
 */
static __always_inline void mmio_write8(u64 addr, u8 value)
{
    *(u8 volatile *) addr = value;
}

/**
 * @brief Write 16-bit value to MMIO address.
 * @param addr MMIO address.
 * @param value Value to write.
 */
static __always_inline void mmio_write16(u64 addr, u16 value)
{
    *(u16 volatile *) addr = value;
}

/**
 * @brief Write 32-bit value to MMIO address.
 * @param addr MMIO address.
 * @param value Value to write.
 */
static __always_inline void mmio_write32(u64 addr, u32 value)
{
    *(u32 volatile *) addr = value;
}

/**
 * @brief Read 8-bit value from MMIO address.
 * @param addr MMIO address.
 * @return Value read.
 */
static __always_inline u8 mmio_read8(u64 addr)
{
    return *(u8 volatile *) addr;
}

/**
 * @brief Read 16-bit value from MMIO address.
 * @param addr MMIO address.
 * @return Value read.
 */
static __always_inline u16 mmio_read16(u64 addr)
{
    return *(u16 volatile *) addr;
}

/**
 * @brief Read 32-bit value from MMIO address.
 * @param addr MMIO address.
 * @return Value read.
 */
static __always_inline u32 mmio_read32(u64 addr)
{
    return *(u32 volatile *) addr;
}

#endif /* AARCH64_IMPL_DRIVERS_MMIO_H */
