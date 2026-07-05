
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

/// @file die.c
/// @brief Kernel Panic Implementation
///
/// This module implements the kernel panic interface.  It is called when
/// the kernel encounters a fatal error and cannot continue.  It prints
/// the panic message and halts the CPU.

#include <kio/die.h>
#include <kio/output.h>

__cold __noreturn void kpanic_impl(char const * file, int line, char const * fmt, ...)
{
    kprintf("\n\n");
    kprintf("*** KERNEL PANIC ***\n");
    kprintf("Location : %s:%d\n", file, line);
    kprintf("Reason   : ");

    va_list ap;
    va_start(ap, fmt);
    vkprintf(fmt, ap);
    va_end(ap);

    kprintf("\n\nSystem halted.\n");
    asm_cpu_halt_forever();
}
