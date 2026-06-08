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

/// @file arch/asm/cpu.h
/// @brief asm CPU architecture contract.

#ifndef ARCH_ASM_CPU_H
#define ARCH_ASM_CPU_H

#include <arch/impl/asm/cpu.h>

static __always_inline void arch_asm_cpu_halt_once(void)
{
    arch_asm_impl_cpu_halt_once();
}

static __always_inline void arch_asm_irq_disable_local(void)
{
    arch_asm_impl_irq_disable_local();
}

static __always_inline void arch_asm_irq_enable_local(void)
{
    arch_asm_impl_irq_enable_local();
}

#endif /* ARCH_ASM_CPU_H */