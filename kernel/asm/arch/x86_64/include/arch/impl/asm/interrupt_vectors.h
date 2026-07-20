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

/// @file arch/impl/asm/interrupt_vectors.h
/// @brief x86_64 interrupt-vector-table load primitive.
///
/// Raw inline-assembly wrapper for LIDT. This is the only permitted site for
/// __asm__ volatile on x86_64 for interrupt vector table loading.
/// Consumed by the interrupts subsystem's x86_64 implementation.

#ifndef X86_64_IMPL_ASM_INTERRUPT_VECTORS_H
#define X86_64_IMPL_ASM_INTERRUPT_VECTORS_H

#include <janus/attributes.h>
#include <janus/types.h>

/// Install the IDT (Interrupt Descriptor Table) via LIDT.
///
/// @param idtr Pointer to a 10-byte pseudo-descriptor { u16 limit; u64 base }.
static __always_inline void arch_asm_load_interrupt_vectors(void const * idtr)
{
    __asm__ volatile("lidt %0" : : "m"(*(u8 const *) idtr) : "memory");
}

#endif /* X86_64_IMPL_ASM_INTERRUPT_VECTORS_H */
