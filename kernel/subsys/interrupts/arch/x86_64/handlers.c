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
/// @brief x86_64 exception dispatch and diagnostic reporting.

#include <arch/internal/interrupts/frame.h>
#include <arch/internal/interrupts/setup.h>
#include <arch/internal/interrupts/vectors.h>
#include <asm/regs.h>
#include <janus/attributes.h>
#include <janus/types.h>
#include <kio/die.h>
#include <kio/output.h>

/// @brief Intel-reserved exception mnemonics indexed by vector number (0–31).
///
/// Entries that correspond to reserved or vendor-specific vectors carry a
/// "(reserved)" placeholder so the array is always fully populated.
static char const * const k_mnemonics[VEC_RESERVED_COUNT] = {
    "#DE Divide Error",
    "#DB Debug",
    "NMI Non-Maskable Interrupt",
    "#BP Breakpoint",
    "#OF Overflow",
    "#BR Bound Range Exceeded",
    "#UD Invalid Opcode",
    "#NM Device Not Available",
    "#DF Double Fault",
    "Coprocessor Segment Overrun",
    "#TS Invalid TSS",
    "#NP Segment Not Present",
    "#SS Stack-Segment Fault",
    "#GP General Protection",
    "#PF Page Fault",
    "(reserved)",
    "#MF x87 Floating-Point",
    "#AC Alignment Check",
    "#MC Machine Check",
    "#XM SIMD Floating-Point",
    "#VE Virtualization",
    "#CP Control Protection",
    "(reserved)",
    "(reserved)",
    "(reserved)",
    "(reserved)",
    "(reserved)",
    "(reserved)",
    "#HV Hypervisor Injection",
    "#VC VMM Communication",
    "#SX Security Exception",
    "(reserved)",
};

/// @brief Print the full CPU register state captured in an interrupt frame.
///
/// Writes the trap frame (RIP, CS, RFLAGS, RSP, SS) and all saved
/// general-purpose registers to the kernel console via kprintf.
///
/// @param frame  Interrupt frame captured by the ISR entry stub in isr.asm.
static void dump_registers(interrupt_frame_t const * frame);

/// @brief Return a human-readable mnemonic for a CPU exception vector.
/// @param vector  CPU exception vector number (0–31).
/// @return Pointer to a static string describing the exception, or a generic
///         "External/Reserved vector" message for vectors outside the reserved range.
char const * interrupts_vector_mnemonic(u64 vector_index)
{
    if (vector_index < VEC_RESERVED_COUNT) {
        return k_mnemonics[vector_index];
    }
    return "External/Reserved vector";
}

//// @brief Dispatch a CPU exception to the kernel panic handler.
////
/// This function is called from the architecture-specific ISR entry stubs
/// in isr.asm. It prints a diagnostic message to the kernel console, including
/// the vector number, mnemonic, error code, and register state.
///
/// @param frame  Pointer to the interrupt frame captured by the ISR entry stub.
__noreturn void interrupts_dispatch(interrupt_frame_t const * frame)
{
    kprintf("\n*** CPU EXCEPTION ***\n");
    kprintf("Vector %lu (%s)  error_code=0x%016lx\n",
            frame->vector,
            interrupts_vector_mnemonic(frame->vector),
            frame->error_code);

    if (frame->vector == VEC_PAGE_FAULT) {
        kprintf("CR2 (faulting address) = 0x%016lx\n", asm_read_fault_address());
    }
    dump_registers(frame);

    kpanic("unhandled CPU exception (vector %lu)", frame->vector);
}

static void dump_registers(interrupt_frame_t const * frame)
{
    kprintf("  RIP=0x%016lx  CS=0x%016lx  RFLAGS=0x%016lx\n", frame->rip, frame->cs, frame->rflags);
    kprintf("  RSP=0x%016lx  SS=0x%016lx\n", frame->rsp, frame->ss);
    kprintf("  RAX=0x%016lx  RBX=0x%016lx  RCX=0x%016lx\n", frame->rax, frame->rbx, frame->rcx);
    kprintf("  RDX=0x%016lx  RSI=0x%016lx  RDI=0x%016lx\n", frame->rdx, frame->rsi, frame->rdi);
    kprintf("  RBP=0x%016lx  R8 =0x%016lx  R9 =0x%016lx\n", frame->rbp, frame->r8, frame->r9);
    kprintf("  R10=0x%016lx  R11=0x%016lx  R12=0x%016lx\n", frame->r10, frame->r11, frame->r12);
    kprintf("  R13=0x%016lx  R14=0x%016lx  R15=0x%016lx\n", frame->r13, frame->r14, frame->r15);
}
