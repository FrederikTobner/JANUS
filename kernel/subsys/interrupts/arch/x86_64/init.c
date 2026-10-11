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

/// @file init.c
/// @brief x86_64 interrupt subsystem entry point.

#include <arch/internal/interrupts/setup.h>
#include <arch/interrupts/init.h>
#include <janus/attributes.h>
#include <janus/errno.h>

/// @brief Initialize the x86_64 interrupt subsystem.
///
/// On x86_64 this function builds and loads a kernel-owned GDT (Global Descriptor Table) and TSS (Task State Segment)
/// with a dedicated IST (Interrupt Stack Table) stack for #DF (Double Fault).
/// It then builds and loads a 256-entry IDT (Interrupt Descriptor Table)
__cold error_t arch_interrupts_init(void)
{
    // GDT, TSS and IST must exist before the IDT references IST1 for #DF.
    gdt_install();
    idt_install();
    return JANUS_OK;
}
