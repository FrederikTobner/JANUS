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
/// @brief AArch64 interrupt frame layout (shared by C and vector_table.S).

#ifndef AARCH64_IMPL_INTERRUPTS_FRAME_H
#define AARCH64_IMPL_INTERRUPTS_FRAME_H

/*
 * Byte offsets and reserved size of the captured frame.
 *
 * These live outside the __ASSEMBLER__ guard so vector_table.S (which is run
 * through the C preprocessor) is the single source of truth together with the
 * struct below. The STATIC_ASSERTs in the C section prove the struct matches
 * these offsets, so a mismatch between the store order in vector_table.S and
 * interrupt_frame_t fails the build rather than corrupting the report.
 */
#define FRAME_OFF_X0       0   ///< x[0] (x0..x30 occupy 0..240, 31 * 8 bytes)
#define FRAME_OFF_SP       248 ///< interrupted stack pointer
#define FRAME_OFF_ELR      256 ///< ELR_EL1
#define FRAME_OFF_SPSR     264 ///< SPSR_EL1
#define FRAME_OFF_ESR      272 ///< ESR_EL1
#define FRAME_OFF_FAR      280 ///< FAR_EL1
#define FRAME_OFF_SOURCE   288 ///< entry-source index (0..15)

/// Bytes reserved on the stack for the frame. The struct is 296 bytes (37 * 8);
/// rounded up to a 16-byte multiple (AAPCS64 stack alignment) with 8 bytes of
/// padding at the top.
#define AARCH64_FRAME_SIZE 304

#ifndef __ASSEMBLER__

#include <janus/attributes.h>
#include <janus/types.h>

/// @brief Full CPU state captured on AArch64 exception entry.
///
/// The field order MUST mirror the store order in vector_table.S (lowest
/// address first): the entry path reserves AARCH64_FRAME_SIZE bytes, saves
/// x0..x30, then the interrupted sp and the syndrome system registers, and
/// finally the synthesised source index. The dispatcher treats the saved
/// stack region as this struct. Do not reorder without updating vector_table.S
/// in lockstep — the STATIC_ASSERTs below guard the offsets.
typedef struct {
    u64 x[31];  ///< General-purpose registers x0..x30 (x30 = LR)
    u64 sp;     ///< Interrupted stack pointer (SP_EL1 at fault time)
    u64 elr;    ///< ELR_EL1  — preferred return address (faulting PC)
    u64 spsr;   ///< SPSR_EL1 — saved PSTATE
    u64 esr;    ///< ESR_EL1  — syndrome (EC[31:26] | ISS)
    u64 far;    ///< FAR_EL1  — faulting address (valid for abort/alignment EC)
    u64 source; ///< Entry-source index 0..15 (group * 4 + type)
} interrupt_frame_t;

STATIC_ASSERT(__offsetof(interrupt_frame_t, x) == FRAME_OFF_X0, "x0 offset mismatch");
STATIC_ASSERT(__offsetof(interrupt_frame_t, sp) == FRAME_OFF_SP, "sp offset mismatch");
STATIC_ASSERT(__offsetof(interrupt_frame_t, elr) == FRAME_OFF_ELR, "elr offset mismatch");
STATIC_ASSERT(__offsetof(interrupt_frame_t, spsr) == FRAME_OFF_SPSR, "spsr offset mismatch");
STATIC_ASSERT(__offsetof(interrupt_frame_t, esr) == FRAME_OFF_ESR, "esr offset mismatch");
STATIC_ASSERT(__offsetof(interrupt_frame_t, far) == FRAME_OFF_FAR, "far offset mismatch");
STATIC_ASSERT(__offsetof(interrupt_frame_t, source) == FRAME_OFF_SOURCE, "source offset mismatch");
STATIC_ASSERT(sizeof(interrupt_frame_t) == 296, "frame must be 37 u64 slots");
STATIC_ASSERT(AARCH64_FRAME_SIZE >= sizeof(interrupt_frame_t), "reserved frame must fit struct");
STATIC_ASSERT((AARCH64_FRAME_SIZE % 16) == 0, "frame size must be 16-byte aligned");

#endif /* __ASSEMBLER__ */

#endif /* AARCH64_IMPL_INTERRUPTS_FRAME_H */
