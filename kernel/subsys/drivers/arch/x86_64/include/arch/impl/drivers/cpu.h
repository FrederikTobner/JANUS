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
/// Provides inline implementations of arch_cpu_* functions.
/// This header is pulled in via include path resolution.

#ifndef X86_64_IMPL_DRIVERS_CPU_H
#define X86_64_IMPL_DRIVERS_CPU_H

#include <janus/attributes.h>

/// @brief Halt the CPU until the next interrupt/event.
static __always_inline void arch_cpu_halt(void)
{
    __asm__ volatile("hlt");
}

/// @brief Disable interrupts.
static __always_inline void arch_cpu_disable_interrupts(void)
{
    __asm__ volatile("cli");
}

/// @brief Enable interrupts.
static __always_inline void arch_cpu_enable_interrupts(void)
{
    __asm__ volatile("sti");
}

#endif /* X86_64_IMPL_DRIVERS_CPU_H */
