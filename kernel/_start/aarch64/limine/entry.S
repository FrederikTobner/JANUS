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

/**
 * This code is executed by Limine after it transfers control.
 * Limine already sets up:
 * - AArch64 execution state
 * - EL1 or EL2 (we handle both cases)
 * - Higher-half paging (maps kernel at virtual address from ELF)
 * - A valid stack (64KB or per stack size request)
 *
 * We transition to EL1t if needed (dropping from EL2, or normalizing SPSel if
 * Limine already handed off at EL1) and call kernel_main. The kernel commits to
 * running at EL1t (SP_EL0 as the current stack) everywhere past this point, so
 * SP_EL1 stays free for use as a dedicated exception-entry stack (see
 * subsys/interrupts and asm/exception_stack.h). Boot context is populated by
 * boot_init inside kernel_main.
 *
 * Note: Limine request structures are defined in common/limine_requests.c
 * for portability across architectures.
 */

.global _start_limine
.extern kernel_main

// AArch64 system register bits
.equ DAIF_IRQ_BIT,  (1 << 7)    // IRQ mask bit
.equ DAIF_FIQ_BIT,  (1 << 6)    // FIQ mask bit
.equ DAIF_MASK_ALL, (0xF << 6)  // Mask all exceptions

// CurrentEL register: bits [3:2] contain current exception level
.equ CURRENT_EL_SHIFT, 2
.equ CURRENT_EL_EL1,   1
.equ CURRENT_EL_EL2,   2

// HCR_EL2: Hypervisor Configuration Register
.equ HCR_EL2_RW,       (1 << 31)  // EL1 is AArch64

// SPSR_EL2: Saved Program Status Register for EL2
.equ SPSR_EL2_D,       (1 << 9)   // Debug exception mask
.equ SPSR_EL2_A,       (1 << 8)   // SError mask
.equ SPSR_EL2_I,       (1 << 7)   // IRQ mask
.equ SPSR_EL2_F,       (1 << 6)   // FIQ mask
// EL1t (SP_EL0 as the current stack) rather than EL1h: the exception-entry path
// (see arch/impl/asm/exception_stack.h and subsys/interrupts) relies on SP_EL1
// being a register the kernel never otherwise uses, so it can serve as a
// dedicated, corruption-resilient exception stack (the aarch64 analog of the
// x86 IST #DF stack). That only holds if normal code runs on SP_EL0.
.equ SPSR_EL2_EL1t,    (4 << 0)   // Return to EL1t (EL1 with SP_EL0)
.equ SPSR_EL2_VALUE,   (SPSR_EL2_D | SPSR_EL2_A | SPSR_EL2_I | SPSR_EL2_F | SPSR_EL2_EL1t)

// SCTLR_EL1: System Control Register for EL1
// Note: RES1 bits handled via ldr pseudo-instruction due to immediate encoding limits
.equ SCTLR_EL1_I,      (1 << 12)   // Instruction cache enable
.equ SCTLR_EL1_C,      (1 << 2)    // Data cache enable
.equ SCTLR_EL1_M,      (1 << 0)    // MMU enable

.section .text.limine_entry, "ax"
.balign 8

/*
 * _start_limine: Limine entry point for AArch64
 *
 * Limine enters here with:
 * - Paging enabled (higher-half mapped per ELF program headers)
 * - Interrupts disabled
 * - A valid stack provided by Limine (64KB by default)
 * - Either EL1 or EL2 depending on firmware
 *
 * We're already running at the higher-half virtual address.
 * We use Limine's stack and page tables.
 */
_start_limine:
    // Mask all asynchronous exceptions (interrupts)
    msr     daifset, #0xF

    // Check current exception level
    mrs     x0, CurrentEL
    lsr     x0, x0, #CURRENT_EL_SHIFT
    cmp     x0, #CURRENT_EL_EL2
    b.ne    .Lel1_entry

    /* 
     * We're at EL2 - need to drop to EL1
     * Configure EL2 to allow EL1 to run in AArch64 mode
     */
    
    // Set EL1 execution state to AArch64
    mov     x0, #HCR_EL2_RW
    msr     hcr_el2, x0
    
    // Enable FP/SIMD at EL1 (clear FPEN traps)
    mov     x0, #(3 << 20)      // FPEN = 0b11: no trapping
    msr     cpacr_el1, x0
    
    // Don't trap FP/SIMD to EL2
    mrs     x0, cptr_el2
    bic     x0, x0, #(1 << 10)  // Clear TFP bit
    msr     cptr_el2, x0
    
    // Initialize SCTLR_EL1 with safe defaults
    // Limine already has MMU/caches configured, preserve that
    // We just read and preserve current SCTLR_EL1 - Limine sets it up
    mrs     x0, sctlr_el1
    // Ensure reserved bits are set (RES1 = 0x30D00800)
    ldr     x1, =0x30D00800
    orr     x0, x0, x1
    msr     sctlr_el1, x0

    // `eret` only changes PC/PSTATE, it never moves data between banked SP
    // registers. Whatever stack Limine gave us is only visible through the
    // *currently active* SP right now (SP_EL2 or SP_EL0, depending on how EL2
    // was entered) — it is not automatically inherited by SP_EL0 at EL1t.
    // Seed SP_EL0 explicitly before dropping, so kernel_main doesn't run on
    // whatever garbage SP_EL0 happens to hold.
    mov     x2, sp
    msr     sp_el0, x2

    // Set up SPSR_EL2 for return to EL1t
    mov     x0, #SPSR_EL2_VALUE
    msr     spsr_el2, x0
    
    // Set ELR_EL2 to our EL1 entry point
    adr     x0, .Lel1_entry
    msr     elr_el2, x0
    
    // Drop to EL1
    eret

.Lel1_entry:
    /*
     * Now running at EL1. Normalize to EL1t (SPSel = 0, SP_EL0 active): the
     * EL2 drop above already lands here in EL1t, but Limine's handoff EL/SPSel
     * when it enters directly at EL1 (the b.ne branch above) is not guaranteed.
     * If we're already on SP_EL0 this is a no-op; otherwise carry the current
     * stack value across the switch so it isn't lost.
     */
    mrs     x0, spsel
    and     x0, x0, #1
    cbz     x0, .Lspsel_ok
    mov     x1, sp
    msr     spsel, #0
    mov     sp, x1
.Lspsel_ok:

    // Limine has already set up the stack for us (now normalized on SP_EL0)
    // Enter kernel_main — boot_init runs inside kernel_main
    bl      kernel_main

    // If kernel_main returns, halt forever
.Lhang:
    wfi                         // Wait For Interrupt (low power halt)
    b       .Lhang

/*
 * NOTE: AArch64 ABI requires 16-byte stack alignment at public function calls. 
 * Limine provides an aligned stack, so we're good.
 */
