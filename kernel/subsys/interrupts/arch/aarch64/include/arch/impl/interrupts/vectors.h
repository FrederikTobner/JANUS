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

/// @file arch/impl/interrupts/vectors.h
/// @brief AArch64 exception-class constants, entry-source indices, and lookups.

#ifndef AARCH64_IMPL_INTERRUPTS_VECTORS_H
#define AARCH64_IMPL_INTERRUPTS_VECTORS_H

#include <janus/types.h>

/// Bit position of the exception class (EC) field within ESR_EL1.
#define ESR_EC_SHIFT             26

/// Extract the 6-bit exception class (EC) from a raw ESR_EL1 value.
#define ESR_EC(esr)              (((esr) >> ESR_EC_SHIFT) & 0x3Full)

/* Exception-class (EC) values decoded by the dispatcher (ESR_EL1[31:26]). */
#define ESR_EC_UNKNOWN           0x00 ///< Unknown reason
#define ESR_EC_ILLEGAL_STATE     0x0E ///< Illegal Execution state
#define ESR_EC_SVC64             0x15 ///< SVC instruction (AArch64)
#define ESR_EC_MSR_MRS           0x18 ///< Trapped MSR/MRS/system instruction
#define ESR_EC_INSTR_ABORT_LOWER 0x20 ///< Instruction Abort, lower EL
#define ESR_EC_INSTR_ABORT_CUR   0x21 ///< Instruction Abort, current EL
#define ESR_EC_PC_ALIGNMENT      0x22 ///< PC alignment fault
#define ESR_EC_DATA_ABORT_LOWER  0x24 ///< Data Abort, lower EL
#define ESR_EC_DATA_ABORT_CUR    0x25 ///< Data Abort, current EL
#define ESR_EC_SP_ALIGNMENT      0x26 ///< SP alignment fault
#define ESR_EC_FP64              0x2C ///< Trapped floating-point exception
#define ESR_EC_SERROR            0x2F ///< SError interrupt
#define ESR_EC_BRK64             0x3C ///< BRK instruction (AArch64)

/// @brief Entry-source index carried from each vector-table slot (0..15).
///
/// Computed as group * 4 + type, mirroring the vector-table layout: the four
/// groups are {Current EL SP0, Current EL SPx, Lower EL AArch64, Lower EL
/// AArch32}, each with the four types {Synchronous, IRQ, FIQ, SError}. The
/// kernel runs at EL1h (SPx) with no EL0, so genuine faults arrive as
/// EXC_SRC_CUR_SPX_SYNC; every other slot is populated defensively.
typedef enum {
    EXC_SRC_CUR_SP0_SYNC = 0,   ///< Current EL, SP0, Synchronous
    EXC_SRC_CUR_SP0_IRQ,        ///< Current EL, SP0, IRQ
    EXC_SRC_CUR_SP0_FIQ,        ///< Current EL, SP0, FIQ
    EXC_SRC_CUR_SP0_SERROR,     ///< Current EL, SP0, SError
    EXC_SRC_CUR_SPX_SYNC,       ///< Current EL, SPx, Synchronous (real faults)
    EXC_SRC_CUR_SPX_IRQ,        ///< Current EL, SPx, IRQ
    EXC_SRC_CUR_SPX_FIQ,        ///< Current EL, SPx, FIQ
    EXC_SRC_CUR_SPX_SERROR,     ///< Current EL, SPx, SError
    EXC_SRC_LOWER_A64_SYNC,     ///< Lower EL, AArch64, Synchronous
    EXC_SRC_LOWER_A64_IRQ,      ///< Lower EL, AArch64, IRQ
    EXC_SRC_LOWER_A64_FIQ,      ///< Lower EL, AArch64, FIQ
    EXC_SRC_LOWER_A64_SERROR,   ///< Lower EL, AArch64, SError
    EXC_SRC_LOWER_A32_SYNC,     ///< Lower EL, AArch32, Synchronous
    EXC_SRC_LOWER_A32_IRQ,      ///< Lower EL, AArch32, IRQ
    EXC_SRC_LOWER_A32_FIQ,      ///< Lower EL, AArch32, FIQ
    EXC_SRC_LOWER_A32_SERROR,   ///< Lower EL, AArch32, SError
    EXC_SRC_COUNT               ///< Number of vector-table entries (16)
} exc_source_t;

/// @brief Map an exception class (EC) to a human-readable mnemonic.
/// @param ec Exception class, i.e. ESR_EL1[31:26].
/// @return A static mnemonic string; a generic fallback for unrecognised classes.
char const * interrupts_ec_mnemonic(u64 ec);

/// @brief Map an entry-source index to a human-readable label.
/// @param source Entry-source index 0..15 (see exc_source_t).
/// @return A static label string; a generic fallback for out-of-range indices.
char const * interrupts_source_label(u64 source);

/// @brief Report whether FAR_EL1 is architecturally meaningful for an EC.
/// @param ec Exception class, i.e. ESR_EL1[31:26].
/// @return true for abort and alignment classes; false otherwise.
bool interrupts_ec_has_fault_address(u64 ec);

#endif /* AARCH64_IMPL_INTERRUPTS_VECTORS_H */
