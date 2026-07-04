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

#ifndef KIO_OUTPUT_H
#define KIO_OUTPUT_H

/// @file output.h
/// @brief Kernel I/O — formatted output and fatal error handling.
///
/// kio is a core-layer service that provides kprintf(), vkprintf(), and
/// kpanic() to any kernel module that needs them. It is deliberately
/// decoupled from the hardware drivers: output is funnelled through a
/// single registered putc callback (kio_register_putc()). The callback
/// is installed once by output_sink_init_early() / output_sink_init() in kmain.
///
/// Before the callback is registered:
///   - kprintf / vkprintf are silent no-ops.
///   - kpanic prints nothing but still halts the CPU.
///
/// Dependency layer (core → lib only):
///   kio depends on:  fmt (formatting)
///   kio must NOT depend on: subsystems (boot, drivers, mm, …)
#include <asm/cpu.h>
#include <janus/attributes.h>
#include <janus/types.h>
#include <janus/va_arg.h>

/// Output callback type that emits one character.
typedef void (*kio_putc_fn)(char c);

/// @brief Register the kernel output callback.
///
/// Called once by output_sink_init_early() before the first kprintf/kpanic.
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

#endif /* KIO_OUTPUT_H */
