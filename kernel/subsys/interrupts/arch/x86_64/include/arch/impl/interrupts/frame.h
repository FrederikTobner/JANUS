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

/// @file arch/impl/interrupts/frame.h
/// @brief x86_64 interrupt frame layout.

#ifndef X86_64_IMPL_INTERRUPTS_FRAME_H
#define X86_64_IMPL_INTERRUPTS_FRAME_H

#include <janus/attributes.h>
#include <janus/types.h>

/// @brief Full CPU state captured on exception entry.
///
/// The field order MUST mirror the push order in isr.asm (lowest address
/// first): the per-vector stub pushes the error code then the vector, the
/// common tail pushes the general-purpose registers (r15 last → lowest
/// address), and the CPU pushes the trap frame (rip..ss) automatically.
/// Do not reorder without updating isr.asm in lockstep.
typedef struct interrupt_frame {
    u64 r15; ///< General-purpose register R15
    u64 r14; ///< General-purpose register R14
    u64 r13; ///< General-purpose register R13
    u64 r12; ///< General-purpose register R12
    u64 r11; ///< General-purpose register R11
    u64 r10; ///< General-purpose register R10
    u64 r9;  ///< General-purpose register R9
    u64 r8;  ///< General-purpose register R8
    u64 rbp; ///< Base pointer
    u64 rdi; ///< Destination index / first integer argument
    u64 rsi; ///< Source index / second integer argument
    u64 rdx; ///< Data register
    u64 rcx; ///< Count register
    u64 rbx; ///< Base register
    u64 rax; ///< Accumulator register

    u64 vector;     ///< Interrupt/exception vector number (0–255)
    u64 error_code; ///< CPU error code, or 0 synthesised for no-error vectors

    u64 rip;    ///< Instruction pointer at the point of fault
    u64 cs;     ///< Code segment selector
    u64 rflags; ///< Flags register
    u64 rsp;    ///< Stack pointer at the point of fault
    u64 ss;     ///< Stack segment selector
} interrupt_frame_t;

#endif /* X86_64_IMPL_INTERRUPTS_FRAME_H */
