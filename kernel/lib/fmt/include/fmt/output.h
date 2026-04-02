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

#ifndef JANUS_FMT_OUTPUT_H
#define JANUS_FMT_OUTPUT_H

#include <janus/attributes.h>
#include <janus/types.h>
#include <janus/va_arg.h>

/** Output sink callback type for formatted output */
typedef void (*fmt_putc_fn)(char c, void * ctx);

/**
 * @brief Formatted output to an arbitrary sink.
 * @param putc Output callback (emits one character)
 * @param ctx Opaque context passed to putc
 * @param fmt Format string
 * @param ... Arguments
 * @return Number of characters written
 */
s32 fmt_to(fmt_putc_fn putc, void * ctx, char const * fmt, ...) __attribute__((format(printf, 3, 4)));

/**
 * @brief Formatted output to an arbitrary sink (va_list variant).
 * @param putc Output callback (emits one character)
 * @param ctx Opaque context passed to putc
 * @param fmt Format string
 * @param ap va_list
 * @return Number of characters written
 */
s32 vfmt_to(fmt_putc_fn putc, void * ctx, char const * fmt, va_list ap);

/**
 * @brief Printf-style formatted output to a buffer (with size limit).
 * @param buf Output buffer
 * @param buflen Buffer size
 * @param fmt Format string
 * @param ... Arguments
 * @return Number of characters written (excluding NUL terminator)
 */
s32 snprintf(char * buf, u64 buflen, char const * fmt, ...) __attribute__((format(printf, 3, 4)));

/**
 * @brief Printf-style formatted output to a buffer (no size limit).
 * @param buf Output buffer
 * @param fmt Format string
 * @param ... Arguments
 * @return Number of characters written (excluding NUL terminator)
 */
s32 sprintf(char * buf, char const * fmt, ...) __attribute__((format(printf, 2, 3)));

#endif /* JANUS_FMT_OUTPUT_H */
