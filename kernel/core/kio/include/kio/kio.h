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

#ifndef KIO_KIO_H
#define KIO_KIO_H

/// @file kio.h
/// @brief Kernel I/O — formatted output and fatal error handling.
///
/// kio is a core-layer service that provides kprintf(), vkprintf(), and
/// kpanic() to any kernel module that needs them. It is deliberately
/// decoupled from the hardware drivers: output is funnelled through a
/// single registered putc callback (kio_register_putc()). The callback
/// is installed once by console_init_early() / console_init() in kmain.
///
/// Before the callback is registered:
///   - kprintf / vkprintf are silent no-ops.
///   - kpanic prints nothing but still halts the CPU.
///
/// Dependency layer (core → lib only):
///   kio depends on:  fmt (formatting)
///   kio must NOT depend on: subsystems (boot, drivers, mm, …)
#include <arch/kio/halt.h>
#include <janus/attributes.h>
#include <janus/types.h>
#include <janus/va_arg.h>

/// Output callback type that emits one character.
typedef void (*kio_putc_fn)(char c);

/// @brief Register the kernel output callback.
///
/// Called once by console_init_early() before the first kprintf/kpanic.
/// Subsequent calls replace the callback (e.g. upgrading from serial-only
/// to serial + TTY).
///
/// @param fn Output function pointer.
void kio_register_putc(kio_putc_fn fn);

/// @brief Kernel printf — formatted output through the registered callback.
///
/// Silent no-op before kio_register_putc() is called.
/// @return Number of characters written (0 before registration)
s32 kprintf(char const * fmtstr, ...) __attribute__((format(printf, 1, 2)));

/// @brief Kernel vprintf — va_list variant of kprintf.
s32 vkprintf(char const * fmtstr, va_list args);

/// @brief Disable interrupts and halt forever.
///
/// Architecture-specific behavior is provided by the arch kio layer.
static __always_inline __noreturn void kio_halt_forever(void)
{
    arch_kio_halt_forever();
}

/// @brief Underlying panic implementation — do not call directly; use kpanic().
///
/// Prints a banner and the diagnostic message if output is available,
/// then halts the CPU permanently.
__cold __noreturn void kpanic_impl(char const * file, int line, char const * fmt, ...)
    __attribute__((format(printf, 3, 4)));

/// @brief Panic with a human-readable message and halt the CPU.
///
/// Injects __FILE__ and __LINE__ automatically so the panic site is always
/// identifiable in the output.
///
/// Example:
/// @code
///     if (err != JANUS_OK) {
///         kpanic("subsystem_init failed: %d", err);
///     }
/// @endcode
#define kpanic(fmt, ...) kpanic_impl(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

#endif /* KIO_KIO_H */
