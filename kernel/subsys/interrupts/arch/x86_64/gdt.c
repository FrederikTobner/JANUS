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

/// @file gdt.c
/// @brief x86_64 GDT (Global Descriptor Table) and TSS (Task State Segment)
///        construction, including the dedicated #DF (Double Fault) IST stack.

#include <arch/internal/interrupts/gdt.h>
#include <arch/internal/interrupts/setup.h>
#include <asm/dt.h>
#include <janus/attributes.h>
#include <janus/types.h>

/// Size of the dedicated double-fault stack (selected via IST1).
#define DF_STACK_SIZE       16384

// Access-byte values.
#define GDT_ACCESS_CODE     0x9A ///< present, DPL 0, code, execute/read
#define GDT_ACCESS_DATA     0x92 ///< present, DPL 0, data, read/write
#define GDT_ACCESS_TSS      0x89 ///< present, DPL 0, available 64-bit TSS

// Flags
#define GDT_FLAGS_LONG_MODE 0x20 ///< Long mode flag of the global descriptor table

static u8 g_df_stack[DF_STACK_SIZE] __aligned(16);
static gdt_entry_t g_gdt[GDT_SLOTS];
static tss_t g_tss;
static gdt_ptr_t g_gdtr;

/// @brief Populate one 8-byte code or data segment descriptor.
///
/// Sets all base and limit fields to zero (ignored in 64-bit mode) and writes
/// the given access byte and flags nibble.
///
/// @param entry   Descriptor slot to fill.
/// @param access  Access byte: present bit, DPL, and segment type.
/// @param flags   Flags/granularity byte (L bit set for 64-bit code, 0 for data).
static void gdt_set_segment(gdt_entry_t * entry, u8 access, u8 flags);

/// @brief Encode a 64-bit TSS base address into a 16-byte GDT system descriptor.
///
/// In 64-bit mode a TSS descriptor occupies two consecutive 8-byte GDT slots.
/// The low slot (@p lo) follows the standard 8-byte layout; the high slot
/// (@p hi) holds base bits [63:32] in its first four bytes, with the rest
/// reserved and zeroed.
///
/// @param lo     Low 8-byte slot of the system descriptor pair.
/// @param hi     High 8-byte slot (receives base bits [63:32]).
/// @param base   Linear base address of the TSS.
/// @param limit  Segment limit for the TSS (typically sizeof(tss_t) - 1).
static void gdt_set_tss(gdt_entry_t * lo, gdt_entry_t * hi, u64 base, u32 limit);

__cold void gdt_install(void)
{
    // g_tss and g_gdt are static (.bss) and therefore already zero-initialised
    // by the C runtime.  Only the fields that deviate from zero need setting.
    // Point IST1 (tss.ist[0]) at the top of the dedicated double-fault stack;
    // the stack grows downward, so the top is the end of the array.
    g_tss.ist[0] = (u64) (g_df_stack + DF_STACK_SIZE);
    g_tss.iomap_base = (u16) sizeof(tss_t);

    // g_gdt[0] is the null descriptor
    gdt_set_segment(&g_gdt[1], GDT_ACCESS_CODE, GDT_FLAGS_LONG_MODE);
    gdt_set_segment(&g_gdt[2], GDT_ACCESS_DATA, 0);
    gdt_set_tss(&g_gdt[3], &g_gdt[4], (u64) &g_tss, (u32) (sizeof(tss_t) - 1));

    g_gdtr.limit = (u16) (sizeof(g_gdt) - 1);
    g_gdtr.base = (u64) &g_gdt;

    asm_load_gdt(&g_gdtr, GDT_SEL_CODE, GDT_SEL_DATA);
    asm_load_tr(GDT_SEL_TSS);
}

static void gdt_set_segment(gdt_entry_t * entry, u8 access, u8 flags)
{
    // Base and limit are ignored for 64-bit code/data segments.
    entry->limit_low = 0;
    entry->base_low = 0;
    entry->base_mid = 0;
    entry->access = access;
    entry->granularity = flags;
    entry->base_high = 0;
}

static void gdt_set_tss(gdt_entry_t * lo, gdt_entry_t * hi, u64 base, u32 limit)
{
    lo->limit_low = (u16) (limit & 0xFFFF);
    lo->base_low = (u16) (base & 0xFFFF);
    lo->base_mid = (u8) ((base >> 16) & 0xFF);
    lo->access = GDT_ACCESS_TSS;
    lo->granularity = (u8) ((limit >> 16) & 0x0F);
    lo->base_high = (u8) ((base >> 24) & 0xFF);

    u32 base_upper = (u32) (base >> 32);
    hi->limit_low = (u16) (base_upper & 0xFFFF);
    hi->base_low = (u16) ((base_upper >> 16) & 0xFFFF);
    hi->base_mid = 0;
    hi->access = 0;
    hi->granularity = 0;
    hi->base_high = 0;
}
