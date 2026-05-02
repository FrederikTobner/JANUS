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

#ifndef KMAIN_CONSOLE_H
#define KMAIN_CONSOLE_H

/// @file console.h
/// @brief Kernel console output.
///
///
/// Provides functions for kernel console output, including formatted printing.

#include <boot/context.h>
#include <janus/types.h>
#include <janus/va_arg.h>

s32 kprintf(char const * fmt, ...) __attribute__((format(printf, 1, 2)));
s32 vkprintf(char const * fmt, va_list ap);

/// @brief Best-effort serial initialization before boot context is available.
///
/// Calls boot_early_params() to obtain address-translation parameters.
/// On Multiboot2 all outputs are zero (x86_64 uses port I/O, so zero is valid).
/// On Limine the real HHDM offset and base addresses are supplied by the bootloader.
/// On AArch64 with Multiboot2 this may silently fail when HHDM data is not yet available.
/// Safe to call multiple times — no-op if serial is already active.
void console_init_early(void);

void console_init(boot_context_t const * boot_context);

#endif // KMAIN_CONSOLE_H