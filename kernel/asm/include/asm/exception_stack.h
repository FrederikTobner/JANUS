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

/// @file asm/exception_stack.h
/// @brief Public asm exception stack entry point.

#ifndef ASM_EXCEPTION_STACK_H
#define ASM_EXCEPTION_STACK_H

#include <asm/capabilities.h>
#include <janus/attributes.h>

#if ASM_CAP_EXCEPTION_STACK_REGISTER
#include <arch/impl/asm/exception_stack.h>

/// @breif set the stack pointer the CPU switches to on exception entry
///
/// On architectures where exception entry selectss a distinct banked stack regiister
/// (aarch64: SP_EL1), this points that register at a valid mapped stack.
/// Architectures that source the exception stack form a descriptor table instead (x86_64: IST1 in the TSS)
/// dont have this capability.
static __always_inline void asm_set_exception_stack(void const * stack_top)
{
    arch_asm_set_exception_stack(stack_top);
}

#endif /* ASM_CAP_EXCEPTION_STACK_REGISTER */

#endif /* ASM_EXCEPTION_STACK_H */
