/*****************************************************************************
 * Copyright (C) 2025 by Frederik Tobner                                     *
 *                                                                           *
 * This file is part of JANUS.                                              *
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

/// @file kio.c
/// @brief Kernel I/O implementation.
///
/// Bridges the fmt library to kernel output hardware via a registered putc
/// callback. Hardware drivers register themselves via kio_register_putc();
/// all kprintf/kpanic calls flow through that single point.
///
/// kpanic halts with interrupts disabled via an inline asm loop so that it
/// never returns, even if no output callback is registered yet.

#include <kio/kio.h>

#include <fmt/output.h>
#include <janus/attributes.h>

// ── Output state ──────────────────────────────────────────────────────────

/// Registered output callback — NULL until kio_register_putc() is called
static kio_putc_fn g_putc = NULL;

/// fmt_to sink adapter: forwards each char to the registered callback
static void kio_fmt_putc(char c, __unused void * ctx)
{
    if (g_putc != NULL) {
        g_putc(c);
    }
}

// ── Public API ─────────────────────────────────────────────────────────────

void kio_register_putc(kio_putc_fn fn)
{
    g_putc = fn;
}

s32 kprintf(char const * fmtstr, ...)
{
    if (g_putc == NULL) {
        return 0;
    }
    va_list ap;
    va_start(ap, fmtstr);
    s32 const ret = vfmt_to(kio_fmt_putc, NULL, fmtstr, ap);
    va_end(ap);
    return ret;
}

s32 vkprintf(char const * fmtstr, va_list args)
{
    if (g_putc == NULL) {
        return 0;
    }
    return vfmt_to(kio_fmt_putc, NULL, fmtstr, args);
}

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

    // Halt with interrupts disabled — architecture-independent infinite loop.
    // We do not call drivers_cpu_halt_forever() to avoid a dependency on
    // the drivers subsystem from the core layer.
    for (;;) {
#if defined(__x86_64__)
        __asm__ volatile("cli; hlt");
#elif defined(__aarch64__)
        __asm__ volatile("msr daifset, #0xf; wfi");
#else
        __asm__ volatile("");
#endif
    }
}
