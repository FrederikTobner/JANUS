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

/// @file interrupts.c
/// @brief Shared logic of the interrupts subsystem.
///
/// Delegates to the architecture-specific arch_interrupts_init().

#include <interrupts/interrupts.h>

#include <arch/interrupts/interrupts.h>
#include <janus/attributes.h>

__cold error_t interrupts_init(void)
{
    return arch_interrupts_init();
}
