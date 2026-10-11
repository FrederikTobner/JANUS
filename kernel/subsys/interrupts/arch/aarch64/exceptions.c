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

/// @file exceptions.c
/// @brief AArch64 EL1 exception vector table installation.

#include <arch/internal/interrupts/setup.h>
#include <asm/exception_stack.h>
#include <asm/interrupt_vectors.h>
#include <janus/attributes.h>
#include <janus/errno.h>
#include <janus/types.h>
#include <kio/die.h>

/// The exception vector table, defined in vector_table.S (2 KiB aligned).
extern u8 janus_vector_table[];

#define EXCEPTION_STACK_SIZE 16384

/// Dedicated stack for exception entry (SP_EL1).
///
/// The kernel main runs at EL1t (using SP_EL0).
/// Exceptions taken to EL1 switch to SP_EL1, which the bootloader leaves pointing at an unmapped low address.
/// This kernel-image .bss stack is guaranteed mapped.
/// This means that the entry path can save the frame without immediately faulting.
/// It also survives a corrupted user stack (SP_EL0), the aarch64 analog of the x86 IST #DF stack.
static u8 g_exception_stack[EXCEPTION_STACK_SIZE] __aligned(16);

__cold error_t exceptions_install(void)
{
    // Ensure we are currently running on the normal execution stack (SP_EL0) before setting up the exception stack.
    if (!asm_exception_stack_inactive()) {
        kpanic("exceptions_install: expected EL1t (SP_EL0 active), but SPSel indicates SP_EL1 is current");
    }

    asm_set_exception_stack(g_exception_stack + sizeof(g_exception_stack));
    asm_load_interrupt_vectors(janus_vector_table);
    return JANUS_OK;
}
