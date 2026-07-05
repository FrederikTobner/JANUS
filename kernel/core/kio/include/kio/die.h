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

#ifndef KIO_DIE_H
#define KIO_DIE_H

/// @file die.h
/// @brief Kernel panic interface.
///
/// This module provides the kpanic() macro, which prints a diagnostic message and halts the CPU.
/// It is intended for unrecoverable errors in the kernel.

#include <janus/attributes.h>
#include <janus/types.h>
#include <janus/va_arg.h>

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

#endif /* KIO_OUTPUT_H */
