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

/// @file interrupts.h
/// @brief Interrupt and CPU-exception handling — public API.
///
/// Architecture contract (arch_interrupts_*) is in
/// <arch/interrupts/interrupts.h>. The public surface is intentionally
/// architecture-agnostic: no x86 concept (IDT, CR2, gate type) appears here.

#ifndef INTERRUPTS_INTERRUPTS_H
#define INTERRUPTS_INTERRUPTS_H

#include <arch/interrupts/interrupts.h>
#include <janus/types.h>

/// @brief Install and activate interrupt/exception handling on the current CPU.
///
/// On x86_64 this function builds and loads a kernel-owned GDT (Global Descriptor Table)
/// plus TSS (Task State Segment) — with a dedicated IST (Interrupt Stack Table)
/// stack for #DF (Double Fault) — then builds and loads a 256-entry IDT
/// (Interrupt Descriptor Table). After this returns, CPU exceptions are routed
/// to handlers that emit a diagnostic panic instead of triple-faulting.
///
/// Must be called exactly once, after console init and before any subsystem
/// that can fault (e.g. the physical memory manager).
///
/// @return JANUS_OK on success; a negative error_t otherwise.
error_t interrupts_init(void);

#endif /* INTERRUPTS_INTERRUPTS_H */
