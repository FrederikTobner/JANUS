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

/// @file arch/impl/interrupts/gdt.h
/// @brief x86_64 GDT (Global Descriptor Table) and TSS (Task State Segment)
///        layout and constants.

#ifndef X86_64_IMPL_INTERRUPTS_GDT_H
#define X86_64_IMPL_INTERRUPTS_GDT_H

#include <janus/attributes.h>
#include <janus/types.h>

/// @brief 8-byte GDT (Global Descriptor Table) descriptor (code/data).
///
/// The TSS (Task State Segment) descriptor is a system descriptor that spans
/// two consecutive entries of this type (16 bytes total) in 64-bit mode.
typedef struct gdt_entry {
    u16 limit_low;  ///< Bits [15:0] of the segment limit
    u16 base_low;   ///< Bits [15:0] of the base address
    u8 base_mid;    ///< Bits [23:16] of the base address
    u8 access;      ///< Access byte (present, DPL, type)
    u8 granularity; ///< Granularity/flags and bits [19:16] of the limit
    u8 base_high;   ///< Bits [31:24] of the base address
} __packed gdt_entry_t;

/// @brief x86_64 TSS (Task State Segment).
///
/// Only the IST (Interrupt Stack Table) and rsp fields are used by JANUS;
/// the rest are present for hardware-layout correctness.
typedef struct tss {
    u32 reserved0;
    u64 rsp0; ///< Stack pointer for privilege level 0
    u64 rsp1; ///< Stack pointer for privilege level 1
    u64 rsp2; ///< Stack pointer for privilege level 2
    u64 reserved1;
    u64 ist[7]; ///< IST stack pointers; ist[0] is the #DF (Double Fault) stack
    u64 reserved2;
    u16 reserved3;
    u16 iomap_base; ///< I/O permission bitmap offset (set to sizeof(tss_t))
} __packed tss_t;

/// @brief Pseudo-descriptor operand for LGDT (loads the GDT register).
typedef struct gdt_ptr {
    u16 limit; ///< Size of the table in bytes minus one
    u64 base;  ///< Linear address of the first GDT entry
} __packed gdt_ptr_t;

/// GDT selectors (offsets into the kernel GDT).
#define GDT_SEL_NULL 0x00 ///< Null descriptor
#define GDT_SEL_CODE 0x08 ///< Kernel code segment
#define GDT_SEL_DATA 0x10 ///< Kernel data segment
#define GDT_SEL_TSS  0x18 ///< Task State Segment descriptor

/// Number of 8-byte slots in the kernel GDT (null, code, data, TSS low/high).
#define GDT_SLOTS    5

STATIC_ASSERT(sizeof(gdt_entry_t) == 8, "GDT entry must be 8 bytes");
STATIC_ASSERT(sizeof(tss_t) == 104, "TSS must be 104 bytes");
STATIC_ASSERT(sizeof(gdt_ptr_t) == 10, "GDT pointer must be 10 bytes");

#endif /* X86_64_IMPL_INTERRUPTS_GDT_H */
