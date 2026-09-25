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
#include <janus/types.h>

#if ASM_CAP_EXCEPTION_STACK_REGISTER
#include <arch/impl/asm/exception_stack.h>

/// @brief Sets the stack pointer the CPU switches to on exception entry
///
/// @details On architectures where exception entry selects a distinct banked stack register
/// (aarch64: SP_EL1), this points that register at a valid mapped stack.
/// Architectures that source the exception stack form a descriptor table instead (x86_64: IST1 in the TSS)
/// don't have this capability.
///
/// @param stack_top Pointer to the top of the exception stack (stack grows downwards)
static __always_inline void asm_set_exception_stack(void const * stack_top)
{
    arch_asm_set_exception_stack(stack_top);
}

/// @brief Checks whether the CPU is currently running with the "current" stack
/// register (e.g. SP_EL0 on aarch64) active, as opposed to the dedicated
/// exception-entry stack register.
///
/// @details Intended as a boot-time invariant check before installing the
/// vector table: the dedicated exception stack this capability sets up is
/// only a genuinely separate, corruption-resilient stack if normal code never
/// runs on it.
///
/// @return true if the CPU is on its normal execution stack, false if it is
/// already on the exception-entry stack register.
static __always_inline bool asm_exception_stack_inactive(void)
{
    return arch_asm_current_sp_is_sp0();
}

#endif /* ASM_CAP_EXCEPTION_STACK_REGISTER */

#endif /* ASM_EXCEPTION_STACK_H */
