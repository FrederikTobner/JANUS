/*****************************************************************************
 * Copyright (C) 2025 by Frederik Tobner                                     *
 *                                                                           *
 * This file is part of JANUS                                                *
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

/// @file kpanic.c
/// @brief Kernel panic implementation.
///
/// kpanic_impl() prints a banner, the source location, and the caller-supplied
/// diagnostic message through every available output channel, then halts
/// the CPU with interrupts disabled.

#include <kmain/kpanic.h>

#include <drivers/cpu.h>
#include <janus/attributes.h>
#include <janus/va_arg.h>
#include <kmain/console.h>

__cold __noreturn void kpanic_impl(char const * file, int line, char const * fmt, ...)
{
    // Ensure at least serial output is available, even if console_init() has
    // not been called yet (e.g. boot_init failure). On AArch64 this may still
    // fail silently if HHDM data was never available.
    console_init_early();

    kprintf("\n\n");
    kprintf("*** KERNEL PANIC ***\n");
    kprintf("Location : %s:%d\n", file, line);
    kprintf("Reason   : ");

    va_list ap;
    va_start(ap, fmt);
    vkprintf(fmt, ap);
    va_end(ap);

    kprintf("\n\nSystem halted.\n");

    drivers_cpu_halt_forever();
}
