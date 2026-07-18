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

/// @file arch/impl/asm/capabilities.h
/// @brief x86_64 asm capability flags.

#ifndef X86_64_IMPL_ASM_CAPABILITIES_H
#define X86_64_IMPL_ASM_CAPABILITIES_H

#define ASM_ARCH_X86_64                1
#define ASM_ARCH_AARCH64               0

#define ASM_CAP_LOCAL_IRQ_CONTROL      1
#define ASM_CAP_IDLE_WAIT_INTERRUPT    1
#define ASM_CAP_TLB_INVALIDATE_PAGE    1
#define ASM_CAP_TLB_INVALIDATE_ALL     0
#define ASM_CAP_PORT_IO                1
#define ASM_CAP_ARCH_SYSREG_ACCESS     1
#define ASM_CAP_INTERRUPT_VECTOR_TABLE 1 // x86_64 uses the IDT for interrupt vectors
#define ASM_CAP_SEGMENT_DESCRIPTORS    1 // x86_64 uses segment descriptors (GDT/IDT/LDT)
#define ASM_CAP_FAULT_ADDRESS_REGISTER 1 // x86_64 has a fault address register (CR2)

// x86_64 uses a unified page table base model (CR3)
#define ASM_CAP_PAGE_TABLE_BASE_MODEL  ASM_CAP_VAL_PAGE_TABLE_BASE_UNIFIED

#endif /* X86_64_IMPL_ASM_CAPABILITIES_H */
