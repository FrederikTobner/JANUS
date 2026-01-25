/*****************************************************************************
 * Copyright (C) 2025 by Frederik Tobner                                     *
 *                                                                           *
 * This file is part of TinyOS.                                              *
 *                                                                           *
 * Permission to use, copy, modify, and distribute this software and its     *
 * documentation under the terms of the GNU Affero General Public License is *
 * hereby granted.                                                           *
 * No representations are made about the suitability of this software for    *
 * any purpose.                                                              *
 * It is provided "as is" without express or implied warranty.              *
 * See the <https://www.gnu.org/licenses/agpl-3.0.en.html>                   *
 * GNU Affero General Public License                                         *
 * License for more details.                                                 *
 ****************************************************************************/

#ifndef ARCH_CPU_H
#define ARCH_CPU_H

#include <tinyos/attributes.h>

/**
 * Halt the CPU until the next interrupt/event.
 *
 * Note: On architectures where halt returns only on interrupts, this can
 * return when interrupts are enabled.
 */
void cpu_halt(void);

/**
 * Disable interrupts (if applicable) and halt forever.
 */
__noreturn void cpu_halt_forever(void);

#endif // ARCH_CPU_H
