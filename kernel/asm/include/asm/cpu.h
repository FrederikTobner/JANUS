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
/// @brief Public asm CPU entry point.

#ifndef ASM_CPU_H
#define ASM_CPU_H

#include <asm/capabilities.h>
#include <janus/attributes.h>
#if ASM_CAP_LOCAL_IRQ_CONTROL || ASM_CAP_IDLE_WAIT_INTERRUPT
#include <arch/impl/asm/cpu.h>
#endif /* ASM_CAP_LOCAL_IRQ_CONTROL || ASM_CAP_IDLE_WAIT_INTERRUPT */

#if ASM_CAP_IDLE_WAIT_INTERRUPT

static __always_inline void asm_cpu_halt_once(void)
{
    arch_asm_cpu_halt_once();
}

#endif /* ASM_CAP_IDLE_WAIT_INTERRUPT */

#if ASM_CAP_LOCAL_IRQ_CONTROL

static __always_inline void asm_cpu_disable_interrupts(void)
{
    arch_asm_irq_disable_local();
}

static __always_inline void asm_cpu_enable_interrupts(void)
{
    arch_asm_irq_enable_local();
}

#endif /* ASM_CAP_LOCAL_IRQ_CONTROL */

#if ASM_CAP_IDLE_WAIT_INTERRUPT && ASM_CAP_LOCAL_IRQ_CONTROL
static __always_inline __noreturn void asm_cpu_halt_forever(void)
{
    asm_cpu_disable_interrupts();
    for (;;) {
        asm_cpu_halt_once();
    }
}
#endif /* ASM_CAP_IDLE_WAIT_INTERRUPT && ASM_CAP_LOCAL_IRQ_CONTROL */

#endif /* ASM_CPU_H */