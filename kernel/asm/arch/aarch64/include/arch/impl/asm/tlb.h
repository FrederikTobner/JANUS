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
/// @brief AArch64 TLB maintenance primitives.
///
/// Raw inline-assembly wrappers for TLBI instructions.
/// This is the only permitted site for __asm__ volatile on AArch64 for TLB ops.
/// Consumed by kernel libraries (e.g. page_tables) that manipulate page tables.

#ifndef AARCH64_IMPL_ASM_TLB_H
#define AARCH64_IMPL_ASM_TLB_H

#include <janus/attributes.h>
#include <janus/types.h>

/// Invalidate TLB entry by VA, EL1, inner shareable (TLBI VALE1IS).
///
/// Invalidates the TLB entry for the given virtual page address in the current
/// ASID, for EL1, broadcast to all CPUs in the inner-shareable domain.
///
/// @param va_page  Virtual address right-shifted by 12 (i.e. va >> 12).
static __always_inline void arch_asm_impl_tlb_invalidate_page(virt_addr_t va)
{
    u64 const va_page = (u64) (va >> 12);
    __asm__ volatile("tlbi vale1is, %0" : : "r"(va_page) : "memory");
}

/// Invalidate all TLB entries for EL1, inner shareable (TLBI VMALLE1IS).
///
/// Flushes all EL1 TLB entries for all ASIDs, broadcast to all CPUs in the
/// inner-shareable domain. Use after a full page table rebuild.
static __always_inline void arch_asm_impl_tlb_invalidate_all(void)
{
    __asm__ volatile("tlbi vmalle1is" ::: "memory");
}

#endif /* AARCH64_IMPL_ASM_TLB_H */