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

/*
 * @file vector_table.S
 * @brief AArch64 EL1 exception vector table and common save/dispatch path.
 *
 * The "table" installed in VBAR_EL1 is code: 16 entries of 0x80 bytes each,
 * one per {source group} x {exception type}. Each slot reserves the frame,
 * saves the interrupted x0/x1, loads its 0..15 source index, and branches to
 * the shared exc_common, which saves the remaining GPRs, snapshots the
 * syndrome system registers, and calls interrupts_dispatch(frame).
 *
 * Field offsets and the frame size come from the shared frame.h so the layout
 * stays in lockstep with interrupt_frame_t (STATIC_ASSERTs there enforce it).
 * The file is named .S so the C preprocessor runs and can include that header.
 */

#include <arch/internal/interrupts/frame.h>

.extern interrupts_dispatch
.global janus_vector_table

.section .text

/*
 * One vector-table slot. Kept tiny (4 instructions, well within 0x80 bytes):
 * the interrupted x0/x1 are stored first so x0 is free to carry the source
 * index into exc_common without losing any interrupted register value.
 */
.macro VECTOR_ENTRY source
    .balign 0x80
    sub     sp, sp, #AARCH64_FRAME_SIZE
    stp     x0, x1, [sp, #FRAME_OFF_X0]
    mov     x0, #\source
    b       exc_common
.endm

// VBAR_EL1 requires the table base to be 2 KiB aligned.
.balign 0x800
janus_vector_table:
    VECTOR_ENTRY 0    // Current EL, SP0,  Synchronous
    VECTOR_ENTRY 1    // Current EL, SP0,  IRQ
    VECTOR_ENTRY 2    // Current EL, SP0,  FIQ
    VECTOR_ENTRY 3    // Current EL, SP0,  SError
    VECTOR_ENTRY 4    // Current EL, SPx,  Synchronous <- real faults land here
    VECTOR_ENTRY 5    // Current EL, SPx,  IRQ
    VECTOR_ENTRY 6    // Current EL, SPx,  FIQ
    VECTOR_ENTRY 7    // Current EL, SPx,  SError
    VECTOR_ENTRY 8    // Lower EL,  AArch64, Synchronous
    VECTOR_ENTRY 9    // Lower EL,  AArch64, IRQ
    VECTOR_ENTRY 10   // Lower EL,  AArch64, FIQ
    VECTOR_ENTRY 11   // Lower EL,  AArch64, SError
    VECTOR_ENTRY 12   // Lower EL,  AArch32, Synchronous
    VECTOR_ENTRY 13   // Lower EL,  AArch32, IRQ
    VECTOR_ENTRY 14   // Lower EL,  AArch32, FIQ
    VECTOR_ENTRY 15   // Lower EL,  AArch32, SError

/*
 * exc_common: shared save + dispatch.
 *
 * On entry: sp points at the reserved frame; x0 holds the 0..15 source index;
 * the interrupted x0/x1 are already stored at FRAME_OFF_X0. Never entered by
 * hardware directly — only reached via the per-slot `b exc_common`.
 */
exc_common:
    // Save the remaining general-purpose registers x2..x30 as interrupted.
    stp     x2,  x3,  [sp, #16]
    stp     x4,  x5,  [sp, #32]
    stp     x6,  x7,  [sp, #48]
    stp     x8,  x9,  [sp, #64]
    stp     x10, x11, [sp, #80]
    stp     x12, x13, [sp, #96]
    stp     x14, x15, [sp, #112]
    stp     x16, x17, [sp, #128]
    stp     x18, x19, [sp, #144]
    stp     x20, x21, [sp, #160]
    stp     x22, x23, [sp, #176]
    stp     x24, x25, [sp, #192]
    stp     x26, x27, [sp, #208]
    stp     x28, x29, [sp, #224]
    str     x30,      [sp, #240]

    // Store the source index (carried in x0, already saved as interrupted).
    str     x0, [sp, #FRAME_OFF_SOURCE]

    // Interrupted SP = value before this handler reserved the frame.
    add     x1, sp, #AARCH64_FRAME_SIZE
    str     x1, [sp, #FRAME_OFF_SP]

    // Snapshot the syndrome system registers at entry (nesting-safe: the
    // dispatcher reads these from the frame rather than re-issuing MRS).
    mrs     x1, elr_el1
    str     x1, [sp, #FRAME_OFF_ELR]
    mrs     x1, spsr_el1
    str     x1, [sp, #FRAME_OFF_SPSR]
    mrs     x1, esr_el1
    str     x1, [sp, #FRAME_OFF_ESR]
    mrs     x1, far_el1
    str     x1, [sp, #FRAME_OFF_FAR]

    // x0 = interrupt_frame_t *; dispatch is __noreturn for fatal faults.
    mov     x0, sp
    bl      interrupts_dispatch

    // Unreached today. Retained as a safety net and a placeholder for a future
    // recoverable-exception restore + eret tail.
1:  b       1b

// Mark the stack as non-executable so the linker does not infer an executable
// stack from this object (the AArch64 analog of the x86 .note.GNU-stack block).
.section .note.GNU-stack, "", %progbits
