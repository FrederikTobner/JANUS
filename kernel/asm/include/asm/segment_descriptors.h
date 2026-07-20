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

/// @file asm/segment_descriptors.h
/// @brief Public asm segment-descriptor-table entry point.

#ifndef ASM_SEGMENT_DESCRIPTORS_H
#define ASM_SEGMENT_DESCRIPTORS_H

#include <asm/capabilities.h>
#include <janus/attributes.h>
#include <janus/types.h>

#if ASM_CAP_SEGMENT_DESCRIPTORS
#include <arch/impl/asm/segment_descriptors.h>

static __always_inline void asm_load_gdt(void const * gdtr, u16 code_sel, u16 data_sel)
{
    arch_asm_load_gdt(gdtr, code_sel, data_sel);
}

static __always_inline void asm_load_tr(u16 sel)
{
    arch_asm_load_tr(sel);
}
#endif /* ASM_CAP_SEGMENT_DESCRIPTORS */

#endif /* ASM_SEGMENT_DESCRIPTORS_H */
