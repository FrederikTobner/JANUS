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

/// @file arch/impl/asm/tlb.h
/// @brief x86_64 TLB invalidation primitives.
///
/// Raw inline-assembly wrappers for INVLPG.
/// This is the only permitted site for __asm__ volatile on x86_64 for TLB ops.
/// Consumed by kernel libraries (e.g. page_tables) that manipulate page tables.

#ifndef X86_64_IMPL_ASM_TLB_H
#define X86_64_IMPL_ASM_TLB_H

#include <janus/attributes.h>
#include <janus/types.h>

/// Invalidate the TLB entry for a single virtual page (INVLPG).
///
/// Removes the TLB entry for the 4 KB page that contains @p va on the local
/// CPU. On SMP systems, a shootdown IPI must be sent to remote CPUs separately
/// (not required in JANUS at this stage — single-core only).
///
/// @param va  Any virtual address within the page to invalidate.
static __always_inline void arch_asm_impl_tlb_invalidate_page(virt_addr_t va)
{
    __asm__ volatile("invlpg (%0)" : : "r"(va) : "memory");
}

#endif /* X86_64_IMPL_ASM_TLB_H */