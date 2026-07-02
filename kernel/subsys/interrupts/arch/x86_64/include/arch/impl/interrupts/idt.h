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

/// @file arch/impl/interrupts/idt.h
/// @brief x86_64 IDT (Interrupt Descriptor Table) layout and constants.

#ifndef X86_64_IMPL_INTERRUPTS_IDT_H
#define X86_64_IMPL_INTERRUPTS_IDT_H

#include <janus/attributes.h>
#include <janus/types.h>

/// @brief One 16-byte x86_64 IDT (Interrupt Descriptor Table) gate descriptor.
typedef struct idt_entry {
    u16 offset_low;  ///< Bits [15:0] of the handler address
    u16 selector;    ///< Code-segment selector for the handler
    u8 ist;          ///< Bits [2:0] IST (Interrupt Stack Table) index (0 = none)
    u8 type_attr;    ///< P (present) | DPL (descriptor privilege level) | gate type
    u16 offset_mid;  ///< Bits [31:16] of the handler address
    u32 offset_high; ///< Bits [63:32] of the handler address
    u32 reserved;    ///< Reserved, must be zero
} __packed idt_entry_t;

/// @brief Pseudo-descriptor operand for LIDT (loads the IDT register).
typedef struct idt_ptr {
    u16 limit; ///< Size of the table in bytes minus one
    u64 base;  ///< Linear address of the first IDT entry
} __packed idt_ptr_t;

/// Number of vectors in the IDT.
#define IDT_ENTRIES          256

/// Present, DPL 0, 64-bit interrupt gate (clears IF, the interrupt-enable flag).
#define IDT_TYPE_INTERRUPT   0x8E
/// Present, DPL 0, 64-bit trap gate (leaves IF unchanged).
#define IDT_TYPE_TRAP        0x8F

/// IST (Interrupt Stack Table) index used by the #DF (Double Fault) gate.
/// 1-based in the gate's ist field; selects tss.ist[0].
#define IDT_IST_DOUBLE_FAULT 1

STATIC_ASSERT(sizeof(idt_entry_t) == 16, "IDT entry must be 16 bytes");
STATIC_ASSERT(sizeof(idt_ptr_t) == 10, "IDT pointer must be 10 bytes");

#endif /* X86_64_IMPL_INTERRUPTS_IDT_H */
