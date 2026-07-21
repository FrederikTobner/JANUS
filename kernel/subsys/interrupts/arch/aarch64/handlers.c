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

/// @file handlers.c
/// @brief AArch64 exception dispatch and diagnostic reporting.

#include <arch/impl/interrupts/frame.h>
#include <arch/impl/interrupts/vectors.h>
#include <arch/internal/interrupts/setup.h>
#include <janus/attributes.h>
#include <janus/types.h>
#include <kio/die.h>
#include <kio/output.h>

/// @brief Human-readable label for each of the 16 entry-source indices.
static char const * const k_source_labels[EXC_SRC_COUNT] = {
    "Current EL SP0 / Synchronous",
    "Current EL SP0 / IRQ",
    "Current EL SP0 / FIQ",
    "Current EL SP0 / SError",
    "Current EL SPx / Synchronous",
    "Current EL SPx / IRQ",
    "Current EL SPx / FIQ",
    "Current EL SPx / SError",
    "Lower EL AArch64 / Synchronous",
    "Lower EL AArch64 / IRQ",
    "Lower EL AArch64 / FIQ",
    "Lower EL AArch64 / SError",
    "Lower EL AArch32 / Synchronous",
    "Lower EL AArch32 / IRQ",
    "Lower EL AArch32 / FIQ",
    "Lower EL AArch32 / SError",
};

/// @brief Print the full CPU register state captured in an interrupt frame.
///
/// Writes x0..x30 (three per line) and the interrupted sp to the kernel console.
///
/// @param frame Interrupt frame captured by the entry path in vector_table.S.
static void dump_registers(interrupt_frame_t const * frame);

char const * interrupts_ec_mnemonic(u64 ec)
{
    // Debug classes (breakpoint / step / watchpoint) share a contiguous range.
    if (ec >= 0x30 && ec <= 0x37) {
        return "Debug event (breakpoint/step/watchpoint)";
    }

    switch (ec) {
    case ESR_EC_UNKNOWN:
        return "Unknown reason";
    case ESR_EC_ILLEGAL_STATE:
        return "Illegal Execution state";
    case ESR_EC_SVC64:
        return "SVC (AArch64)";
    case ESR_EC_MSR_MRS:
        return "Trapped MSR/MRS/system instruction";
    case ESR_EC_INSTR_ABORT_LOWER:
        return "Instruction Abort, lower EL";
    case ESR_EC_INSTR_ABORT_CUR:
        return "Instruction Abort, current EL";
    case ESR_EC_PC_ALIGNMENT:
        return "PC alignment fault";
    case ESR_EC_DATA_ABORT_LOWER:
        return "Data Abort, lower EL";
    case ESR_EC_DATA_ABORT_CUR:
        return "Data Abort, current EL";
    case ESR_EC_SP_ALIGNMENT:
        return "SP alignment fault";
    case ESR_EC_FP64:
        return "Trapped floating-point exception";
    case ESR_EC_SERROR:
        return "SError interrupt";
    case ESR_EC_BRK64:
        return "BRK instruction (AArch64)";
    default:
        return "Unrecognised exception class";
    }
}

char const * interrupts_source_label(u64 source)
{
    if (source < EXC_SRC_COUNT) {
        return k_source_labels[source];
    }
    return "Unknown source";
}

bool interrupts_ec_has_fault_address(u64 ec)
{
    switch (ec) {
    case ESR_EC_INSTR_ABORT_LOWER:
    case ESR_EC_INSTR_ABORT_CUR:
    case ESR_EC_PC_ALIGNMENT:
    case ESR_EC_DATA_ABORT_LOWER:
    case ESR_EC_DATA_ABORT_CUR:
    case ESR_EC_SP_ALIGNMENT:
        return true;
    default:
        return false;
    }
}

__noreturn void interrupts_dispatch(interrupt_frame_t const * frame)
{
    u64 ec = ESR_EC(frame->esr);

    kprintf("\n*** CPU EXCEPTION ***\n");
    kprintf("Source %llu (%s)\n", (unsigned long long) frame->source, interrupts_source_label(frame->source));
    kprintf("ESR_EL1=0x%016llx  EC=0x%02llx (%s)\n",
            (unsigned long long) frame->esr,
            (unsigned long long) ec,
            interrupts_ec_mnemonic(ec));
    if (interrupts_ec_has_fault_address(ec)) {
        kprintf("FAR_EL1 (faulting address) = 0x%016llx\n", (unsigned long long) frame->far);
    }
    kprintf(
        "ELR_EL1=0x%016llx  SPSR_EL1=0x%016llx\n", (unsigned long long) frame->elr, (unsigned long long) frame->spsr);
    dump_registers(frame);

    kpanic("unhandled CPU exception (source %llu, EC 0x%02llx)",
           (unsigned long long) frame->source,
           (unsigned long long) ec);
}

static void dump_registers(interrupt_frame_t const * frame)
{
    for (u32 i = 0; i < 31; ++i) {
        kprintf("  x%-2u=0x%016llx", i, (unsigned long long) frame->x[i]);
        if ((i % 3) == 2) {
            kprintf("\n");
        }
    }
    kprintf("  sp =0x%016llx\n", (unsigned long long) frame->sp);
}
