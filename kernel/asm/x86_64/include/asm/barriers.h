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

/// @file asm/barriers.h
/// @brief x86_64 memory barrier primitives.
///
/// Raw inline-assembly wrappers for MFENCE, LFENCE, and SFENCE instructions.
/// This is the only permitted site for __asm__ volatile on x86_64 for barriers.
/// Required before IRQ, PCI, and HDA device work.

#ifndef ASM_X86_64_BARRIERS_H
#define ASM_X86_64_BARRIERS_H

#include <janus/attributes.h>

/// Full memory fence — orders all prior loads and stores (MFENCE).
static __always_inline void asm_mfence(void)
{
    __asm__ volatile("mfence" ::: "memory");
}

/// Load fence — orders all prior load operations (LFENCE).
static __always_inline void asm_lfence(void)
{
    __asm__ volatile("lfence" ::: "memory");
}

/// Store fence — orders all prior store operations (SFENCE).
static __always_inline void asm_sfence(void)
{
    __asm__ volatile("sfence" ::: "memory");
}

#endif /* ASM_X86_64_BARRIERS_H */
