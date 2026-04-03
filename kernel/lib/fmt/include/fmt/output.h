/*****************************************************************************
 * Copyright (C) 2025 by Frederik Tobner                                     *
 *                                                                           *
 * This file is part of JANUS                                              *
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

/// @file output.h
/// @brief Formatted output functions
#ifndef JANUS_FMT_OUTPUT_H
#define JANUS_FMT_OUTPUT_H

#include <janus/attributes.h>
#include <janus/types.h>
#include <janus/va_arg.h>

/// Output callback type for fmt_to and vfmt_to
typedef void (*fmt_putc_fn)(char c, void * context);

/**
 * @brief Formatted output to an arbitrary sink.
 * @param putc Output callback (emits one character)
 * @param context Opaque context passed to putc
 * @param format Format string
 * @param ... Arguments
 * @return Number of characters written
 */
s32 fmt_to(fmt_putc_fn putc, void * context, char const * format, ...) __attribute__((format(printf, 3, 4)));

/**
 * @brief Formatted output to an arbitrary sink (va_list variant).
 * @param putc Output callback (emits one character)
 * @param context Opaque context passed to putc
 * @param format Format string
 * @param ap va_list
 * @return Number of characters written
 */
s32 vfmt_to(fmt_putc_fn putc, void * context, char const * format, va_list ap);

/**
 * @brief Printf-style formatted output to a buffer (with size limit).
 * @param out_buffer Output buffer
 * @param buflen Buffer size
 * @param format Format string
 * @param ... Arguments
 * @return Number of characters written (excluding NULL terminator)
 */
s32 snprintf(char * out_buffer, u64 buflen, char const * format, ...) __attribute__((format(printf, 3, 4)));

/**
 * @brief Printf-style formatted output to a buffer (no size limit).
 * @param out_buffer Output buffer
 * @param format Format string
 * @param ... Arguments
 * @return Number of characters written (excluding NUL terminator)
 */
s32 sprintf(char * out_buffer, char const * format, ...) __attribute__((format(printf, 2, 3)));

#endif /* JANUS_FMT_OUTPUT_H */
