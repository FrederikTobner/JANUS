
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

/// @file output.c
/// @brief Kernel I/O implementation.
///
/// Bridges the fmt library to kernel output hardware via a registered putc
/// callback. Hardware drivers register themselves via kio_register_putc();
/// all kprintf/kpanic calls flow through that single point.
///
/// kpanic halts via an architecture-specific backend selected by CMake.

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
