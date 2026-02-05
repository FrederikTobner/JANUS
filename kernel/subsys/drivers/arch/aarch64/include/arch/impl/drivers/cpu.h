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
 * @file arch/impl/drivers/cpu.h
 * @brief AArch64 CPU control implementation.
 *
 * Provides inline implementations of arch_cpu_* functions.
 * This header is pulled in via include path resolution.
 */

#ifndef AARCH64_IMPL_DRIVERS_CPU_H
#define AARCH64_IMPL_DRIVERS_CPU_H

#include <janus/attributes.h>

/**
 * @brief Halt the CPU until the next interrupt/event.
 *
 * Uses WFI (Wait For Interrupt) instruction on AArch64.
 */
static __always_inline void arch_cpu_halt(void)
{
    __asm__ volatile("wfi");
}

/**
 * @brief Disable interrupts.
 *
 * Sets the DAIF interrupt mask bits to disable IRQ and FIQ.
 */
static __always_inline void arch_cpu_disable_interrupts(void)
{
    __asm__ volatile("msr daifset, #0xF" ::: "memory");
}

/**
 * @brief Enable interrupts.
 *
 * Clears the DAIF interrupt mask bits to enable IRQ and FIQ.
 */
static __always_inline void arch_cpu_enable_interrupts(void)
{
    __asm__ volatile("msr daifclr, #0xF" ::: "memory");
}

/**
 * @brief Disable interrupts and halt forever.
 *
 * This function never returns and is used for unrecoverable errors.
 */
static __always_inline __noreturn void arch_cpu_halt_forever(void)
{
    arch_cpu_disable_interrupts();
    for (;;) {
        arch_cpu_halt();
    }
}

#endif /* AARCH64_IMPL_DRIVERS_CPU_H */
