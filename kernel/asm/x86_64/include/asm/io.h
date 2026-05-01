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

/// @file asm/io.h
/// @brief x86_64 port I/O primitives.
///
/// Raw inline-assembly wrappers for IN/OUT port instructions.
/// This is the only permitted site for __asm__ volatile on x86_64 for port I/O.
/// Consumed by subsystem Tier 3 headers and kernel libraries.

#ifndef ASM_X86_64_IO_H
#define ASM_X86_64_IO_H

#include <janus/attributes.h>
#include <janus/types.h>

/// Write a byte to an I/O port.
///
/// @param port  The I/O port address.
/// @param value The byte value to write.
static __always_inline void asm_io_outb(u16 port, u8 value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

/// Read a byte from an I/O port.
///
/// @param port The I/O port address.
/// @return     The byte read from the port.
static __always_inline u8 asm_io_inb(u16 port)
{
    u8 ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

#endif /* ASM_X86_64_IO_H */
