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

/// @file arch/impl/asm/regs.h
/// @brief AArch64 system register access primitives.
///
/// Raw inline-assembly wrappers for MRS/MSR instructions on named system registers.
/// This is the only permitted site for __asm__ volatile on AArch64 for register access.
/// Consumed by kernel libraries (e.g. page_tables) that need to read/write MMU registers.

#ifndef AARCH64_IMPL_ASM_REGS_H
#define AARCH64_IMPL_ASM_REGS_H

#include <janus/attributes.h>
#include <janus/types.h>

/// Read the faulting address (FAR_EL1) register.
///
/// @return Faulting virtual address.
static __always_inline u64 arch_asm_read_fault_address(void)
{
    u64 val;
    __asm__ volatile("mrs %0, far_el1" : "=r"(val));
    return val;
}

/// Read the kernel-mode page table base register (TTBR1_EL1).
///
/// @return Physical address of the L0 page table used for EL1 kernel mappings.
static __always_inline u64 arch_asm_read_ttbr1_el1(void)
{
    u64 val;
    __asm__ volatile("mrs %0, ttbr1_el1" : "=r"(val));
    return val;
}

/// Write the kernel-mode page table base register (TTBR1_EL1).
///
/// @param val Physical address of the L0 page table (must be page-aligned).
static __always_inline void arch_asm_write_ttbr1_el1(u64 val)
{
    __asm__ volatile("msr ttbr1_el1, %0" : : "r"(val) : "memory");
}

/// Read the user-mode page table base register (TTBR0_EL1).
///
/// @return Physical address of the L0 page table used for EL0/user mappings.
static __always_inline u64 arch_asm_read_ttbr0_el1(void)
{
    u64 val;
    __asm__ volatile("mrs %0, ttbr0_el1" : "=r"(val));
    return val;
}

/// Write the user-mode page table base register (TTBR0_EL1).
///
/// @param val Physical address of the L0 page table (must be page-aligned).
static __always_inline void arch_asm_write_ttbr0_el1(u64 val)
{
    __asm__ volatile("msr ttbr0_el1, %0" : : "r"(val) : "memory");
}

#endif /* AARCH64_IMPL_ASM_REGS_H */