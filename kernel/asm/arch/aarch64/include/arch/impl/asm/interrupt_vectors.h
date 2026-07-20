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
/// @brief AArch64 interrupt-vector-table load primitive.
///
/// Raw inline-assembly wrapper for installing the EL1 exception vector table.
/// This is the only permitted site for __asm__ volatile on AArch64 for
/// exception vector table installation.
/// Consumed by the interrupts subsystem's AArch64 implementation.

#ifndef AARCH64_IMPL_ASM_INTERRUPT_VECTORS_H
#define AARCH64_IMPL_ASM_INTERRUPT_VECTORS_H

#include <janus/attributes.h>

/// Install the EL1 exception vector table.
///
/// @param vector_table 2 KiB-aligned virtual address of the 16-entry vector table.
static __always_inline void arch_asm_load_interrupt_vectors(void const * vector_table)
{
    __asm__ volatile("msr vbar_el1, %0\n\t"
                     "isb"
                     :
                     : "r"(vector_table)
                     : "memory");
}

#endif /* AARCH64_IMPL_ASM_INTERRUPT_VECTORS_H */
