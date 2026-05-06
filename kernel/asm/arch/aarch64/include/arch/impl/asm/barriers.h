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

/// Data synchronisation barrier — complete all memory accesses before continuing (DSB SY).
static __always_inline void arch_asm_impl_barrier_full(void)
{
    __asm__ volatile("dsb sy" ::: "memory");
}

/// Instruction synchronisation barrier — flush the pipeline and refetch instructions (ISB).
static __always_inline void arch_asm_impl_barrier_load(void)
{
    __asm__ volatile("isb" ::: "memory");
}

/// Data memory barrier — order memory accesses without completing them (DMB SY).
static __always_inline void arch_asm_impl_barrier_store(void)
{
    __asm__ volatile("dmb sy" ::: "memory");
}

#endif /* AARCH64_IMPL_ASM_BARRIERS_H */