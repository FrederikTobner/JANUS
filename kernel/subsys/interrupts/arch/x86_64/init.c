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

#include <arch/interrupts/interrupts.h>
#include <arch/internal/interrupts/setup.h>
#include <janus/attributes.h>
#include <janus/errno.h>

__cold error_t arch_interrupts_init(void)
{
    // GDT + TSS + IST must exist before the IDT references IST1 for #DF.
    gdt_install();
    idt_install();
    return JANUS_OK;
}
