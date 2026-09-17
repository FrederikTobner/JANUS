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

/// @file arch/impl/asm/barriers.h
/// @brief AArch64 memory barrier primitives.
///
/// Raw inline-assembly wrappers for DSB, ISB, and DMB instructions.
/// This is the only permitted site for __asm__ volatile on AArch64 for barriers.
/// Required before IRQ, PCI, and device driver work.

#ifndef AARCH64_IMPL_ASM_BARRIERS_H
#define AARCH64_IMPL_ASM_BARRIERS_H

#include <janus/attributes.h>

/// @brief Data synchronisation barrier
/// @details Completes all memory accesses before continuing by utilizing the DSB SY assemlbly instruction.
static __always_inline void arch_asm_barrier_full(void)
{
    __asm__ volatile("dsb sy" ::: "memory");
}

/// @brief Instruction synchronisation barrier
/// @details flushes the pipeline and refetch instructions by utilizing the ISB instruction.
static __always_inline void arch_asm_barrier_load(void)
{
    __asm__ volatile("isb" ::: "memory");
}

/// @brief Data memory barrier
/// @details Orders memory accesses without completing them by utilizing the DMB SY instruction.
static __always_inline void arch_asm_barrier_store(void)
{
    __asm__ volatile("dmb sy" ::: "memory");
}

#endif /* AARCH64_IMPL_ASM_BARRIERS_H */
