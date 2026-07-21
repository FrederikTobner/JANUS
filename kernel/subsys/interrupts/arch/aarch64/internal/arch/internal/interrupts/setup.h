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

/// @file arch/internal/interrupts/setup.h
/// @brief AArch64 interrupts subsystem-private setup declarations.
///
/// Internal to the interrupts AArch64 implementation; never included externally.

#ifndef AARCH64_INTERNAL_INTERRUPTS_SETUP_H
#define AARCH64_INTERNAL_INTERRUPTS_SETUP_H

#include <arch/impl/interrupts/frame.h>
#include <janus/attributes.h>
#include <janus/types.h>

/// @brief Install the EL1 exception vector table (writes VBAR_EL1).
///
/// @return JANUS_OK (cannot fail in v1: the table is a static symbol).
error_t exceptions_install(void);

/// @brief Common C dispatch for every vector entry; reports state and panics.
///
/// Called from exc_common (vector_table.S) with a pointer to the captured
/// frame; reports the fault over kprintf and panics. Never returns.
///
/// @param frame Snapshot of CPU state captured by the vector_table.S entry path.
__noreturn void interrupts_dispatch(interrupt_frame_t const * frame);

#endif /* AARCH64_INTERNAL_INTERRUPTS_SETUP_H */
