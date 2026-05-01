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

/// @file arch/impl/drivers/io.h
/// @brief x86_64 port I/O operations.
///
/// Thin wrappers that forward outb/inb to the asm layer primitives.

#ifndef X86_64_IMPL_DRIVERS_IO_H
#define X86_64_IMPL_DRIVERS_IO_H

#include <asm/io.h>
#include <janus/attributes.h>
#include <janus/types.h>

/// Write a byte to an I/O port.
///
/// @param port The I/O port address.
/// @param value The byte value to write.
static __always_inline void outb(u16 port, u8 value)
{
    asm_io_outb(port, value);
}

/// Read a byte from an I/O port.
///
/// @param port The I/O port address.
/// @return The byte read from the port.
static __always_inline u8 inb(u16 port)
{
    return asm_io_inb(port);
}

#endif /* X86_64_IMPL_DRIVERS_IO_H */
