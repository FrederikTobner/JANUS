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

/// @file arch/impl/asm/dt.h
/// @brief x86_64 descriptor-table load primitives.
///
/// Raw inline-assembly wrappers for LIDT/LGDT/LTR. This is the only permitted
/// site for __asm__ volatile on x86_64 for descriptor-table loading.
/// Consumed by the interrupts subsystem's x86_64 implementation.

#ifndef X86_64_IMPL_ASM_DT_H
#define X86_64_IMPL_ASM_DT_H

#include <janus/attributes.h>
#include <janus/types.h>

/// Install the IDT (Interrupt Descriptor Table) via LIDT.
///
/// @param idtr Pointer to a 10-byte pseudo-descriptor with a u16 limit and a u64 base }.
static __always_inline void arch_asm_impl_load_interrupt_vectors(void const * idtr)
{
    __asm__ volatile("lidt %0" : : "m"(*(u8 const *) idtr) : "memory");
}

/// Load the GDT (Global Descriptor Table) via LGDT and reload every segment
/// register, performing a far return to reload CS (the code segment register).
///
/// @param gdtr     Pointer to a 10-byte pseudo-descriptor { u16 limit; u64 base }.
/// @param code_sel Code-segment selector to load into CS.
/// @param data_sel Data-segment selector to load into DS/ES/SS/FS/GS.
static __always_inline void arch_asm_impl_load_gdt(void const * gdtr, u16 code_sel, u16 data_sel)
{
    __asm__ volatile("lgdt %[gdtr]\n\t"
                     "mov %w[data], %%ds\n\t"
                     "mov %w[data], %%es\n\t"
                     "mov %w[data], %%ss\n\t"
                     "mov %w[data], %%fs\n\t"
                     "mov %w[data], %%gs\n\t"
                     "pushq %[code]\n\t"         /* new CS selector            */
                     "leaq 1f(%%rip), %%rax\n\t" /* return RIP                 */
                     "pushq %%rax\n\t"
                     "lretq\n\t" /* far return reloads CS:RIP  */
                     "1:\n\t"
                     :
                     : [gdtr] "m"(*(u8 const *) gdtr), [code] "r"((u64) code_sel), [data] "r"((u32) data_sel)
                     : "rax", "memory");
}

/// Load the Task Register (TR) via LTR with the TSS (Task State Segment) selector.
///
/// @param sel The TSS selector
static __always_inline void arch_asm_impl_load_tr(u16 sel)
{
    __asm__ volatile("ltr %0" : : "r"(sel));
}

#endif /* X86_64_IMPL_ASM_DT_H */
