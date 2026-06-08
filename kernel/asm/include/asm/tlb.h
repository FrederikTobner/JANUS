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

/// @file asm/tlb.h
/// @brief Public asm TLB entry point.

#ifndef ASM_TLB_H
#define ASM_TLB_H

#include <arch/asm/tlb.h>
#include <asm/capabilities.h>

static __always_inline void asm_tlb_invalidate_page(virt_addr_t va)
{
    arch_asm_tlb_invalidate_page(va);
}

/* Compatibility aliases for existing call sites. */
#if ASM_ARCH_X86_64
static __always_inline void asm_tlb_invlpg(virt_addr_t va)
{
    asm_tlb_invalidate_page(va);
}
#endif

#if ASM_ARCH_AARCH64
static __always_inline void asm_tlbi_vale1is(u64 va_page)
{
    arch_asm_tlb_invalidate_page((virt_addr_t) (va_page << 12));
}

static __always_inline void asm_tlbi_vmalle1is(void)
{
    arch_asm_tlb_invalidate_all();
}
#endif

#endif /* ASM_TLB_H */