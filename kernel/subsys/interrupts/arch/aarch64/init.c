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

/// @file init.c
/// @brief AArch64 interrupt subsystem entry point.

#include <arch/internal/interrupts/setup.h>
#include <arch/interrupts/init.h>
#include <janus/attributes.h>
#include <janus/errno.h>

/// @brief Initialize the AArch64 interrupt subsystem.
///
/// On AArch64 this function sets up the exception vector table and configures the necessary CPU registers to handle
/// interrupts and exceptions appropriately.
__cold error_t arch_interrupts_init(void)
{
    return exceptions_install();
}
