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

/// @file asm/dt.h
/// @brief Public asm descriptor-table entry point.
///
/// Wrappers for loading the x86_64 descriptor tables (IDT/GDT) and the task
/// register. These are x86_64-only; the header body is empty on other
/// architectures, so it is safe to include only from x86_64 code paths.

#ifndef ASM_DT_H
#define ASM_DT_H

#include <arch/asm/dt.h>
#include <asm/capabilities.h>
#include <janus/attributes.h>
#include <janus/types.h>

#if ASM_ARCH_X86_64
static __always_inline void asm_load_idt(void const * idtr)
{
    arch_asm_load_idt(idtr);
}

static __always_inline void asm_load_gdt(void const * gdtr, u16 code_sel, u16 data_sel)
{
    arch_asm_load_gdt(gdtr, code_sel, data_sel);
}

static __always_inline void asm_load_tr(u16 sel)
{
    arch_asm_load_tr(sel);
}
#endif

#endif /* ASM_DT_H */
