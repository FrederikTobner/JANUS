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
/// @brief Public asm barrier entry point.

#ifndef ASM_BARRIERS_H
#define ASM_BARRIERS_H

#include <arch/asm/barriers.h>
#include <asm/capabilities.h>

static __always_inline void asm_barrier_full(void)
{
    arch_asm_barrier_full();
}

static __always_inline void asm_barrier_load(void)
{
    arch_asm_barrier_load();
}

static __always_inline void asm_barrier_store(void)
{
    arch_asm_barrier_store();
}

/* Compatibility aliases for existing call sites. */
#if ASM_ARCH_X86_64
static __always_inline void asm_mfence(void)
{
    asm_barrier_full();
}

static __always_inline void asm_lfence(void)
{
    asm_barrier_load();
}

static __always_inline void asm_sfence(void)
{
    asm_barrier_store();
}
#endif

#if ASM_ARCH_AARCH64
static __always_inline void asm_dsb(void)
{
    asm_barrier_full();
}

static __always_inline void asm_isb(void)
{
    asm_barrier_load();
}

static __always_inline void asm_dmb(void)
{
    asm_barrier_store();
}
#endif

#endif /* ASM_BARRIERS_H */