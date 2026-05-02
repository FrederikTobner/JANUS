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

#ifndef KMAIN_KPANIC_H
#define KMAIN_KPANIC_H

/// @file kpanic.h
/// @brief Kernel panic — unrecoverable fatal error handler.
///
/// @c kpanic() prints a diagnostic message with file/line information, then
/// halts the CPU permanently.  It is the correct response to any situation
/// from which the kernel cannot safely recover.
///
/// Usage:
/// @code
///     if (boot_init(&desc.boot) != JANUS_OK) {
///         kpanic("boot_init failed");
///     }
///     if (result < 0) {
///         kpanic("subsystem_init failed: error %d", result);
///     }
/// @endcode
///
/// The macro injects @c __FILE__ and @c __LINE__ automatically so that the
/// panic site is always identifiable in the output.

// kpanic_impl and the kpanic() macro are now provided by the kio core module.
#include <kio/kio.h>

/// @brief Panic with a human-readable message and halt.
///
/// Expands to a call to kpanic_impl() with the current file and line injected.
#define kpanic(fmt, ...) kpanic_impl(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

#endif /* KMAIN_KPANIC_H */
