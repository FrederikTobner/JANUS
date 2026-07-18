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

/// @file asm/capabilities.h
/// @brief Compile-time asm capability surface.

#ifndef ASM_CAPABILITIES_H
#define ASM_CAPABILITIES_H

#include <arch/asm/capabilities.h>
#include <janus/attributes.h>

/*
 * Validation block:
 * Every architecture must define all ASM_ARCH_* and ASM_CAP_* macros,
 * and each value must be 0 or 1.
 */

#define ASM_VALIDATE_BOOL_MACRO(name) STATIC_ASSERT(((name) == 0) || ((name) == 1), #name " must be 0 or 1")

/* Required architecture selectors */

// Build target is x86_64 (1) or not (0).
#ifndef ASM_ARCH_X86_64
#error "Missing required macro: ASM_ARCH_X86_64"
#else
ASM_VALIDATE_BOOL_MACRO(ASM_ARCH_X86_64);
#endif

// Build target is AArch64 (1) or not (0).
#ifndef ASM_ARCH_AARCH64
#error "Missing required macro: ASM_ARCH_AARCH64"
#else
ASM_VALIDATE_BOOL_MACRO(ASM_ARCH_AARCH64);
#endif

/* Required capability flags */

// Local interrupt mask/unmask operations are available.
#ifndef ASM_CAP_LOCAL_IRQ_CONTROL
#error "Missing required macro: ASM_CAP_LOCAL_IRQ_CONTROL"
#else
ASM_VALIDATE_BOOL_MACRO(ASM_CAP_LOCAL_IRQ_CONTROL);
#endif

// CPU supports interrupt-wait idle/halt-once behavior.
#ifndef ASM_CAP_IDLE_WAIT_INTERRUPT
#error "Missing required macro: ASM_CAP_IDLE_WAIT_INTERRUPT"
#else
ASM_VALIDATE_BOOL_MACRO(ASM_CAP_IDLE_WAIT_INTERRUPT);
#endif

// Per-page local TLB invalidation is available.
#ifndef ASM_CAP_TLB_INVALIDATE_PAGE
#error "Missing required macro: ASM_CAP_TLB_INVALIDATE_PAGE"
#else
ASM_VALIDATE_BOOL_MACRO(ASM_CAP_TLB_INVALIDATE_PAGE);
#endif

#ifndef ASM_CAP_TLB_INVALIDATE_ALL
#error "Missing required macro: ASM_CAP_TLB_INVALIDATE_ALL"
#else
ASM_VALIDATE_BOOL_MACRO(ASM_CAP_TLB_INVALIDATE_ALL);
#endif

// Port I/O instructions (in/out style) are available.
#ifndef ASM_CAP_PORT_IO
#error "Missing required macro: ASM_CAP_PORT_IO"
#else
ASM_VALIDATE_BOOL_MACRO(ASM_CAP_PORT_IO);
#endif

// Privileged architecture-specific system register helpers are available.
#ifndef ASM_CAP_ARCH_SYSREG_ACCESS
#error "Missing required macro: ASM_CAP_ARCH_SYSREG_ACCESS"
#else
ASM_VALIDATE_BOOL_MACRO(ASM_CAP_ARCH_SYSREG_ACCESS);
#endif

#ifndef ASM_CAP_INTERRUPT_VECTOR_TABLE
#error "Missing required macro: ASM_CAP_INTERRUPT_VECTOR_TABLE"
#else
ASM_VALIDATE_BOOL_MACRO(ASM_CAP_INTERRUPT_VECTOR_TABLE);
#endif

#ifndef ASM_CAP_SEGMENT_DESCRIPTORS
#error "Missing required macro: ASM_CAP_SEGMENT_DESCRIPTORS"
#else
ASM_VALIDATE_BOOL_MACRO(ASM_CAP_SEGMENT_DESCRIPTORS);
#endif

#ifndef ASM_CAP_FAULT_ADDRESS_REGISTER
#error "Missing required macro: ASM_CAP_FAULT_ADDRESS_REGISTER"
#else
ASM_VALIDATE_BOOL_MACRO(ASM_CAP_FAULT_ADDRESS_REGISTER);
#endif

#ifndef ASM_CAP_PAGE_TABLE_BASE_MODEL
#error "Missing required macro: ASM_CAP_PAGE_TABLE_BASE_MODEL"
#else
STATIC_ASSERT((ASM_CAP_PAGE_TABLE_BASE_MODEL == ASM_CAP_VAL_PAGE_TABLE_BASE_UNIFIED) ||
                  (ASM_CAP_PAGE_TABLE_BASE_MODEL == ASM_CAP_VAL_PAGE_TABLE_BASE_SPLIT),
              "ASM_CAP_PAGE_TABLE_BASE_MODEL must be either ASM_CAP_VAL_PAGE_TABLE_BASE_UNIFIED or "
              "ASM_CAP_VAL_PAGE_TABLE_BASE_SPLIT");
#endif

#undef ASM_VALIDATE_BOOL_MACRO

// Single architecture build validation: exactly one ASM_ARCH_* macro must be set to 1.
#if ((ASM_ARCH_X86_64 + ASM_ARCH_AARCH64) != 1)
#error "Exactly one ASM_ARCH_* macro must be set to 1"
#endif

#endif /* ASM_CAPABILITIES_H */
