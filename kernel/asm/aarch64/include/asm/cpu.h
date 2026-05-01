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

/// @file asm/cpu.h
/// @brief AArch64 CPU control primitives.
///
/// Raw inline-assembly wrappers for privileged CPU instructions.
/// This is the only permitted site for __asm__ volatile on AArch64 for CPU control.
/// Consumed by subsystem Tier 3 headers and kernel libraries.

#ifndef ASM_AARCH64_CPU_H
#define ASM_AARCH64_CPU_H

#include <janus/attributes.h>

/// Wait For Interrupt — suspend execution until an interrupt arrives (WFI).
static __always_inline void asm_cpu_wfi(void)
{
    __asm__ volatile("wfi");
}

/// Disable all interrupts by setting the DAIF mask (IRQ, FIQ, SError, Debug).
static __always_inline void asm_cpu_daif_set(void)
{
    __asm__ volatile("msr daifset, #0xF" ::: "memory");
}

/// Enable all interrupts by clearing the DAIF mask (IRQ, FIQ, SError, Debug).
static __always_inline void asm_cpu_daif_clr(void)
{
    __asm__ volatile("msr daifclr, #0xF" ::: "memory");
}

#endif /* ASM_AARCH64_CPU_H */
