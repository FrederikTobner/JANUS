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

/// @file idt.c
/// @brief x86_64 IDT (Interrupt Descriptor Table) construction and loading.

#include <arch/internal/interrupts/gdt.h>
#include <arch/internal/interrupts/idt.h>
#include <arch/internal/interrupts/setup.h>
#include <arch/internal/interrupts/vectors.h>
#include <asm/dt.h>
#include <janus/attributes.h>
#include <janus/types.h>

/// Table of entry-stub addresses, defined in isr.asm.
extern u64 isr_stub_table[IDT_ENTRIES];

static idt_entry_t g_idt[IDT_ENTRIES];
static idt_ptr_t g_idtr;

/// @brief Write one 16-byte interrupt gate descriptor into the IDT.
///
/// @param vec      Vector number (0–255); used as the index into g_idt.
/// @param handler  Virtual address of the ISR entry stub.
/// @param type     Gate type/attribute byte (e.g. IDT_TYPE_INTERRUPT).
/// @param ist      IST index: 0 = disabled, 1–7 = use tss.ist[index - 1].
static void idt_set_gate(u8 vec, u64 handler, u8 type, u8 ist);

__cold void idt_install(void)
{
    for (u32 v = 0; v < IDT_ENTRIES; ++v) {
        u8 ist = (u8) ((v == VEC_DOUBLE_FAULT) ? IDT_IST_DOUBLE_FAULT : 0);
        idt_set_gate((u8) v, isr_stub_table[v], (u8) IDT_TYPE_INTERRUPT, ist);
    }

    g_idtr.limit = (u16) (sizeof(g_idt) - 1);
    g_idtr.base = (u64) &g_idt;
    asm_load_interrupt_vectors(&g_idtr);
}

static void idt_set_gate(u8 vec, u64 handler, u8 type, u8 ist)
{
    idt_entry_t * entry = &g_idt[vec];
    entry->offset_low = (u16) (handler & 0xFFFF);
    entry->offset_mid = (u16) ((handler >> 16) & 0xFFFF);
    entry->offset_high = (u32) (handler >> 32);
    entry->selector = (u16) GDT_SEL_CODE;
    entry->ist = (u8) (ist & 0x7);
    entry->type_attr = type;
    entry->reserved = 0;
}
