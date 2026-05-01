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

/// @file arch/impl/drivers/cpu.h
/// @brief x86_64 CPU control implementation.
///
/// Thin wrappers that forward arch_cpu_* to the asm layer primitives.
/// This header is pulled in via include path resolution.

#ifndef X86_64_IMPL_DRIVERS_CPU_H
#define X86_64_IMPL_DRIVERS_CPU_H

#include <asm/cpu.h>
#include <janus/attributes.h>

/// @brief Halt the CPU until the next interrupt/event.
static __always_inline void arch_cpu_halt(void)
{
    asm_cpu_hlt();
}

/// @brief Disable interrupts.
static __always_inline void arch_cpu_disable_interrupts(void)
{
    asm_cpu_cli();
}

/// @brief Enable interrupts.
static __always_inline void arch_cpu_enable_interrupts(void)
{
    asm_cpu_sti();
}

#endif /* X86_64_IMPL_DRIVERS_CPU_H */
