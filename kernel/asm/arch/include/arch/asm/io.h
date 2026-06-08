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

/// @file arch/asm/io.h
/// @brief asm port I/O architecture contract.

#ifndef ARCH_ASM_IO_H
#define ARCH_ASM_IO_H

#include <arch/impl/asm/io.h>

static __always_inline void arch_asm_io_outb(u16 port, u8 value)
{
    arch_asm_impl_io_outb(port, value);
}

static __always_inline u8 arch_asm_io_inb(u16 port)
{
    return arch_asm_impl_io_inb(port);
}

#endif /* ARCH_ASM_IO_H */