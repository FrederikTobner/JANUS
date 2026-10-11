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

/// @file arch/impl/asm/exception_stack.h
/// @brief AArch64 exception stack pointer register (SP_EL1) access.
///
/// Raw inline assembly wrapper. The kernel runs at EL1t (using SP_EL0);
/// exceptions taken to EL1 switch to SP_EL1 automatically, which the bootloader leaves uninitialized.
/// The wrapper points SP_EL1 at a valid, mapped stack before the vector table is installed.
///
/// SP_EL1 is NOT accessible via 'MSR SP_EL1' from EL1 (only EL2/EL3 have that access; it traps at EL1).
/// Instead we momentarily make SP_EL1 the active stack via `SPSel`, write it with a plain `MOV SP`, and switch
/// straight back to SP_EL0. The wrapper is called from the kernel entry point before the vector table is installed,
/// so no exceptions can occur during the switch.

#ifndef AARCH64_IMPL_ASM_EXCEPTION_STACK_H
#define AARCH64_IMPL_ASM_EXCEPTION_STACK_H

#include <janus/attributes.h>
#include <janus/types.h>

/// @brief Point SP_EL1 (the exception-entry stack) at @p stack_top.
///
/// Must be called while running at EL1t (SP_EL0 active). The kernel entry point runs at EL1t, so this is safe to call
/// from there.
static __always_inline void arch_asm_set_exception_stack(void const * stack_top)
{
    __asm__ volatile("msr spsel, #1\n" // Switch to SP_EL1
                     "mov sp, %0\n"    // Write SP_EL1
                     "msr spsel, #0\n" // Switch back to SP_EL0
                     :                 /* no outputs */
                     : "r"(stack_top)
                     : "memory");
}

/// @brief Check whether the current stack is SP_EL0 (i.e. running at EL1t).
///
/// Used as a boot-time sanity check: the dedicated-exception-stack scheme this
/// header implements only makes sense if normal code runs on SP_EL0, leaving
/// SP_EL1 free as a corruption-resilient landing stack for exception entry.
static __always_inline bool arch_asm_current_sp_is_sp0(void)
{
    u64 spsel;
    __asm__ volatile("mrs %0, spsel" : "=r"(spsel));
    return (spsel & 1u) == 0;
}

#endif /* AARCH64_IMPL_ASM_EXCEPTION_STACK_H */