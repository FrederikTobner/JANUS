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

/// @file arch/impl/asm/regs.h
/// @brief x86_64 control register access primitives.
///
/// Raw inline-assembly wrappers for MOV to/from control registers.
/// This is the only permitted site for __asm__ volatile on x86_64 for
/// control register access.
/// Consumed by kernel libraries (e.g. page_tables) that need to read/write
/// the page directory base register.

#ifndef X86_64_IMPL_ASM_REGS_H
#define X86_64_IMPL_ASM_REGS_H

#include <janus/attributes.h>
#include <janus/types.h>

/// Read the Page Directory Base Register (CR3).
///
/// @return Physical address of the current PML4 table, plus PCID bits in [11:0].
static __always_inline u64 arch_asm_impl_read_cr3(void)
{
    u64 val;
    __asm__ volatile("mov %%cr3, %0" : "=r"(val));
    return val;
}

/// Write the Page Directory Base Register (CR3).
///
/// Writing CR3 flushes all non-global TLB entries.
///
/// @param val Physical address of the PML4 table (must be 4 KB aligned).
static __always_inline void arch_asm_impl_write_cr3(u64 val)
{
    __asm__ volatile("mov %0, %%cr3" : : "r"(val) : "memory");
}

#endif /* X86_64_IMPL_ASM_REGS_H */