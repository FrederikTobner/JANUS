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

/// @file arch/impl/interrupts/vectors.h
/// @brief x86_64 exception vector numbers and mnemonic lookup.

#ifndef X86_64_IMPL_INTERRUPTS_VECTORS_H
#define X86_64_IMPL_INTERRUPTS_VECTORS_H

#include <janus/types.h>

#define VEC_INVALID_OPCODE 6  ///< #UD — Invalid Opcode (Undefined Instruction)
#define VEC_DEVICE_NA      7  ///< #NM — Device Not Available (no math coprocessor)
#define VEC_DOUBLE_FAULT   8  ///< #DF — Double Fault
#define VEC_GP_FAULT       13 ///< #GP — General Protection fault
#define VEC_PAGE_FAULT     14 ///< #PF — Page Fault

/// Number of Intel-reserved exception vectors (0–31).
#define VEC_RESERVED_COUNT 32

/// @brief Map a vector number to its Intel exception mnemonic.
///
/// @param vector The interrupt/exception vector number.
/// @return A static mnemonic string for vectors 0–31; a generic
///         "External/Reserved vector" label (covering hardware IRQs and
///         reserved vectors) otherwise.
char const * interrupts_vector_mnemonic(u64 vector);

#endif /* X86_64_IMPL_INTERRUPTS_VECTORS_H */
