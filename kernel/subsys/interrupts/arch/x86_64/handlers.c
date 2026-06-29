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

#include <arch/impl/interrupts/frame.h>
#include <arch/impl/interrupts/vectors.h>
#include <arch/internal/interrupts/setup.h>
#include <asm/regs.h>
#include <janus/attributes.h>
#include <janus/types.h>
#include <kio/kio.h>

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
/// @param f  Interrupt frame captured by the ISR entry stub in isr.asm.
static void dump_registers(interrupt_frame_t const * f);

char const * interrupts_vector_mnemonic(u64 vector)
{
    if (vector < VEC_RESERVED_COUNT) {
        return k_mnemonics[vector];
    }
    return "External/Reserved vector";
}

__noreturn void interrupts_dispatch(interrupt_frame_t const * frame)
{
    kprintf("\n*** CPU EXCEPTION ***\n");
    kprintf("Vector %llu (%s)  error_code=0x%016llx\n",
            (unsigned long long) frame->vector,
            interrupts_vector_mnemonic(frame->vector),
            (unsigned long long) frame->error_code);

    if (frame->vector == VEC_PAGE_FAULT) {
        kprintf("CR2 (faulting address) = 0x%016llx\n", (unsigned long long) asm_read_cr2());
    }
    dump_registers(frame);

    kpanic("unhandled CPU exception (vector %llu)", (unsigned long long) frame->vector);
}

static void dump_registers(interrupt_frame_t const * f)
{
    kprintf("  RIP=0x%016llx  CS=0x%016llx  RFLAGS=0x%016llx\n",
            (unsigned long long) f->rip,
            (unsigned long long) f->cs,
            (unsigned long long) f->rflags);
    kprintf("  RSP=0x%016llx  SS=0x%016llx\n", (unsigned long long) f->rsp, (unsigned long long) f->ss);
    kprintf("  RAX=0x%016llx  RBX=0x%016llx  RCX=0x%016llx\n",
            (unsigned long long) f->rax,
            (unsigned long long) f->rbx,
            (unsigned long long) f->rcx);
    kprintf("  RDX=0x%016llx  RSI=0x%016llx  RDI=0x%016llx\n",
            (unsigned long long) f->rdx,
            (unsigned long long) f->rsi,
            (unsigned long long) f->rdi);
    kprintf("  RBP=0x%016llx  R8 =0x%016llx  R9 =0x%016llx\n",
            (unsigned long long) f->rbp,
            (unsigned long long) f->r8,
            (unsigned long long) f->r9);
    kprintf("  R10=0x%016llx  R11=0x%016llx  R12=0x%016llx\n",
            (unsigned long long) f->r10,
            (unsigned long long) f->r11,
            (unsigned long long) f->r12);
    kprintf("  R13=0x%016llx  R14=0x%016llx  R15=0x%016llx\n",
            (unsigned long long) f->r13,
            (unsigned long long) f->r14,
            (unsigned long long) f->r15);
}
