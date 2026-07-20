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

/// @file asm/interrupt_vectors.h
/// @brief Public asm interrupt-vector-table entry point.

#ifndef ASM_INTERRUPT_VECTORS_H
#define ASM_INTERRUPT_VECTORS_H

#include <asm/capabilities.h>
#include <janus/attributes.h>

#if ASM_CAP_INTERRUPT_VECTOR_TABLE
#include <arch/impl/asm/interrupt_vectors.h>

static __always_inline void asm_load_interrupt_vectors(void const * table)
{
    arch_asm_load_interrupt_vectors(table);
}
#endif /* ASM_CAP_INTERRUPT_VECTOR_TABLE */

#endif /* ASM_INTERRUPT_VECTORS_H */
