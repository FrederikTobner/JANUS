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

/// @file arch/internal/interrupts/setup.h
/// @brief x86_64 interrupts subsystem-private setup declarations.
///
/// Internal to the interrupts x86_64 implementation; never included externally.

#ifndef X86_64_INTERNAL_INTERRUPTS_SETUP_H
#define X86_64_INTERNAL_INTERRUPTS_SETUP_H

#include <arch/impl/interrupts/frame.h>
#include <janus/attributes.h>

/// @brief Build and load the kernel GDT (Global Descriptor Table) and TSS (Task
///        State Segment), wiring IST1 (Interrupt Stack Table entry 1) to the
///        dedicated #DF (Double Fault) stack.
void gdt_install(void);

/// @brief Populate all 256 IDT (Interrupt Descriptor Table) gates from the stub
///        table and load it via LIDT.
void idt_install(void);

/// @brief Common C dispatch for every ISR (Interrupt Service Routine) stub.
///
/// Called from isr_common (isr.asm) with a pointer to the saved frame; reports
/// the fault over kprintf/serial and halts. Never returns.
///
/// @param frame Snapshot of CPU state captured by the entry stub.
__noreturn void interrupts_dispatch(interrupt_frame_t const * frame);

#endif /* X86_64_INTERNAL_INTERRUPTS_SETUP_H */
