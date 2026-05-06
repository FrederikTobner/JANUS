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

/// @file arch/asm/regs.h
/// @brief asm register architecture contract.

#ifndef ARCH_ASM_REGS_H
#define ARCH_ASM_REGS_H

#include <arch/asm/capabilities.h>
#include <arch/impl/asm/regs.h>

#if ASM_ARCH_X86_64
static __always_inline u64 arch_asm_read_cr3(void)
{
    return arch_asm_impl_read_cr3();
}

static __always_inline void arch_asm_write_cr3(u64 val)
{
    arch_asm_impl_write_cr3(val);
}
#endif

#if ASM_ARCH_AARCH64
static __always_inline u64 arch_asm_read_ttbr1_el1(void)
{
    return arch_asm_impl_read_ttbr1_el1();
}

static __always_inline void arch_asm_write_ttbr1_el1(u64 val)
{
    arch_asm_impl_write_ttbr1_el1(val);
}

static __always_inline u64 arch_asm_read_ttbr0_el1(void)
{
    return arch_asm_impl_read_ttbr0_el1();
}

static __always_inline void arch_asm_write_ttbr0_el1(u64 val)
{
    arch_asm_impl_write_ttbr0_el1(val);
}
#endif

#endif /* ARCH_ASM_REGS_H */