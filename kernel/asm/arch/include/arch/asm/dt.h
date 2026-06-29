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

#if ASM_ARCH_X86_64
#include <arch/impl/asm/dt.h>

static __always_inline void arch_asm_load_idt(void const * idtr)
{
    arch_asm_impl_load_idt(idtr);
}

static __always_inline void arch_asm_load_gdt(void const * gdtr, u16 code_sel, u16 data_sel)
{
    arch_asm_impl_load_gdt(gdtr, code_sel, data_sel);
}

static __always_inline void arch_asm_load_tr(u16 sel)
{
    arch_asm_impl_load_tr(sel);
}
#endif

#endif /* ARCH_ASM_DT_H */
