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
 * We transition to EL1 if needed and call kernel_main.
 * Boot context is populated by boot_init inside kernel_main.
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
.equ SPSR_EL2_EL1h,    (5 << 0)   // Return to EL1h (EL1 with SP_EL1)
.equ SPSR_EL2_VALUE,   (SPSR_EL2_D | SPSR_EL2_A | SPSR_EL2_I | SPSR_EL2_F | SPSR_EL2_EL1h)

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
    
    // Set up SPSR_EL2 for return to EL1h
    mov     x0, #SPSR_EL2_VALUE
    msr     spsr_el2, x0
    
    // Set ELR_EL2 to our EL1 entry point
    adr     x0, .Lel1_entry
    msr     elr_el2, x0
    
    // Drop to EL1
    eret

.Lel1_entry:
    /*
     * Now running at EL1
     * Limine has already set up the stack for us
     * Enter kernel_main — boot_init runs inside kernel_main
     */

    // Enter kernel 
    bl      kernel_main

    // If kernel_main returns, halt forever
.Lhang:
    wfi                         // Wait For Interrupt (low power halt)
    b       .Lhang

/*
 * NOTE: AArch64 ABI requires 16-byte stack alignment at public function calls. 
 * Limine provides an aligned stack, so we're good.
 */
