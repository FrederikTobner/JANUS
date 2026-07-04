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

#ifndef KMAIN_OUTPUT_SINK_H
#define KMAIN_OUTPUT_SINK_H

/// @file kmain/output_sink.h
/// @brief Kernel output sink — fans kio output to serial and console.
///
/// This module does exactly one thing: register a putc callback with kio
/// that forwards each character to whichever drivers are active (serial,
/// text console, or both).  It has no input path, no line discipline, and
/// no cursor of its own.

#include <boot/context.h>

/// @brief Best-effort serial initialization before the boot context is available.
///
/// Calls boot_early_params() to obtain address-translation parameters.
/// Safe to call multiple times — no-op if serial is already active.
void output_sink_init_early(void);

/// @brief Initialize the output sink after the boot context is available.
///
/// Initializes serial and console drivers based on the boot context, then
/// re-registers the putc callback to include any newly available outputs.
///
/// @param boot_context  Pointer to the boot context.
void output_sink_init(boot_context_t const * boot_context);

#endif /* KMAIN_OUTPUT_SINK_H */
