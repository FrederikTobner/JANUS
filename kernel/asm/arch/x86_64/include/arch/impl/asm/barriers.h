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
/// @brief x86_64 memory barrier primitives.
///
/// Raw inline-assembly wrappers for MFENCE, LFENCE, and SFENCE instructions.
/// This is the only permitted site for __asm__ volatile on x86_64 for barriers.
/// Required before IRQ, PCI, and HDA device work.

#ifndef X86_64_IMPL_ASM_BARRIERS_H
#define X86_64_IMPL_ASM_BARRIERS_H

#include <janus/attributes.h>

/// @brief Full memory fence 
/// @details Orders all prior loads and stores by utilizing the MFENCE instruction.
static __always_inline void arch_asm_barrier_full(void)
{
    __asm__ volatile("mfence" ::: "memory");
}

/// @brief Load fence
/// @details Orders all prior load operations by utilizing the LFENCE instruction.
static __always_inline void arch_asm_barrier_load(void)
{
    __asm__ volatile("lfence" ::: "memory");
}

/// @brief Store fence 
/// @details Orders all prior store operations by utilizing the SFENCE instruction.
static __always_inline void arch_asm_barrier_store(void)
{
    __asm__ volatile("sfence" ::: "memory");
}

#endif /* X86_64_IMPL_ASM_BARRIERS_H */
