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

/// @file arch/interrupts/init.h
/// @brief Interrupt handling architecture contract.
///
/// Declares arch_interrupts_* functions implemented in arch/<ARCH>/.

#ifndef ARCH_INTERRUPTS_INIT_H
#define ARCH_INTERRUPTS_INIT_H

#include <janus/types.h>

/// @brief Architecture-specific interrupt initialisation.
///
/// Implemented per architecture in arch/<ARCH>/. x86_64 installs the GDT
/// (Global Descriptor Table), TSS (Task State Segment), and IDT (Interrupt
/// Descriptor Table); aarch64 will install a VBAR_EL1 (Vector Base Address
/// Register, EL1) exception vector table.
///
/// @return JANUS_OK on success; a negative error_t otherwise.
error_t arch_interrupts_init(void);

#endif /* ARCH_INTERRUPTS_INIT_H */
