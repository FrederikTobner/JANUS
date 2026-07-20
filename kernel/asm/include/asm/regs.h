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

/// @file asm/regs.h
/// @brief Public asm register entry point.

#ifndef ASM_REGS_H
#define ASM_REGS_H

#include <arch/impl/asm/regs.h>
#include <asm/capabilities.h>

#if ASM_CAP_FAULT_ADDRESS_REGISTER
static __always_inline u64 asm_read_fault_address(void)
{
    return arch_asm_read_fault_address();
}
#endif /* ASM_CAP_FAULT_ADDRESS_REGISTER */

#if ASM_CAP_PAGE_TABLE_BASE_MODEL == ASM_CAP_VAL_PAGE_TABLE_BASE_UNIFIED
static __always_inline u64 asm_read_cr3(void)
{
    return arch_asm_read_cr3();
}

static __always_inline void asm_write_cr3(u64 val)
{
    arch_asm_write_cr3(val);
}
#endif /* ASM_CAP_PAGE_TABLE_BASE_MODEL == ASM_CAP_VAL_PAGE_TABLE_BASE_UNIFIED */

#if ASM_CAP_PAGE_TABLE_BASE_MODEL == ASM_CAP_VAL_PAGE_TABLE_BASE_SPLIT
static __always_inline u64 asm_read_ttbr1_el1(void)
{
    return arch_asm_read_ttbr1_el1();
}

static __always_inline void asm_write_ttbr1_el1(u64 val)
{
    arch_asm_write_ttbr1_el1(val);
}

static __always_inline u64 asm_read_ttbr0_el1(void)
{
    return arch_asm_read_ttbr0_el1();
}

static __always_inline void asm_write_ttbr0_el1(u64 val)
{
    arch_asm_write_ttbr0_el1(val);
}
#endif /* ASM_CAP_PAGE_TABLE_BASE_MODEL == ASM_CAP_VAL_PAGE_TABLE_BASE_SPLIT */

#endif /* ASM_REGS_H */