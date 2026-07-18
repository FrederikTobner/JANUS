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

/// @file arch/asm/dt.h
/// @brief asm descriptor-table architecture contract.

#ifndef ARCH_ASM_DT_H
#define ARCH_ASM_DT_H

#include <arch/asm/capabilities.h>
#include <arch/impl/asm/dt.h>

#if ASM_CAP_INTERRUPT_VECTOR_TABLE
static __always_inline void arch_asm_load_interrupt_vectors(void const * vector_table)
{
    arch_asm_impl_load_interrupt_vectors(vector_table);
}
#endif /* ASM_CAP_INTERRUPT_VECTOR_TABLE */

#if ASM_CAP_SEGMENT_DESCRIPTORS
static __always_inline void arch_asm_load_gdt(void const * gdtr, u16 code_sel, u16 data_sel)
{
    arch_asm_impl_load_gdt(gdtr, code_sel, data_sel);
}

static __always_inline void arch_asm_load_tr(u16 sel)
{
    arch_asm_impl_load_tr(sel);
}
#endif /* ASM_CAP_SEGMENT_DESCRIPTORS */

#endif /* ARCH_ASM_DT_H */
