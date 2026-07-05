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

/// @file exceptions.c
/// @brief aarch64 exception handling (not yet implemented).

#include <arch/interrupts/init.h>
#include <janus/attributes.h>
#include <janus/errno.h>

/// @brief aarch64 exception initialisation — currently a no-op.
///
/// TODO: allocate a 2 KiB-aligned exception vector table, populate its 16
/// entries, and install it via MSR VBAR_EL1 (Vector Base Address Register, EL1),
/// using an asm wrapper added under ASM_ARCH_AARCH64. Returning JANUS_OK keeps
/// boot behaviour on aarch64 identical to before the interrupts subsystem
/// existed, so the shared kmain init sequence links and runs unchanged.
__cold error_t arch_interrupts_init(void)
{
    return JANUS_OK;
}
