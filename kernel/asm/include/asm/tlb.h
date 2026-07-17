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

#if ASM_CAP_TLB_INVALIDATE_PAGE
static __always_inline void asm_tlb_invalidate_page(virt_addr_t va)
{
    arch_asm_tlb_invalidate_page(va);
}
#endif /* ASM_CAP_TLB_INVALIDATE_PAGE */

#if ASM_CAP_TLB_INVALIDATE_ALL
static __always_inline void asm_tlb_invalidate_all(void)
{
    arch_asm_tlb_invalidate_all();
}
#endif /* ASM_CAP_TLB_INVALIDATE_ALL */

#endif /* ASM_TLB_H */