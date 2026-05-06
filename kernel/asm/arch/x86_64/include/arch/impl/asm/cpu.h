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

/// @file arch/impl/asm/cpu.h
/// @brief x86_64 CPU control primitives.
///
/// Raw inline-assembly wrappers for privileged CPU instructions.
/// This is the only permitted site for __asm__ volatile on x86_64 for CPU control.
/// Consumed by subsystem Tier 3 headers and kernel libraries.

#ifndef X86_64_IMPL_ASM_CPU_H
#define X86_64_IMPL_ASM_CPU_H

#include <janus/attributes.h>

/// Halt the CPU until the next interrupt (HLT).
static __always_inline void arch_asm_impl_cpu_halt_once(void)
{
    __asm__ volatile("hlt");
}

/// Disable external interrupts (CLI).
static __always_inline void arch_asm_impl_irq_disable_local(void)
{
    __asm__ volatile("cli" ::: "memory");
}

/// Enable external interrupts (STI).
static __always_inline void arch_asm_impl_irq_enable_local(void)
{
    __asm__ volatile("sti" ::: "memory");
}

#endif /* X86_64_IMPL_ASM_CPU_H */