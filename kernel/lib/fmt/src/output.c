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

/// @file output.c
/// @brief Kernel formatted output: number/string formatting helpers (internal)
#include <fmt/output.h>
#include <janus/types.h>
#include <janus/va_arg.h>

/// @brief Format an unsigned integer with the given base and options, outputting via putc.
/// @param putc Output callback function
/// @param context Opaque context passed to putc
/// @param value Unsigned integer value to format
/// @param base Numeric base (e.g., 10 for decimal, 16 for hexadecimal)
/// @param upper If true, use uppercase letters for bases > 10; otherwise, use lowercase
/// @param width Minimum field width (pad with 'pad' character if necessary)
/// @param pad Padding character (e.g., ' ' or '0')
/// @param left If true, left-justify the output; otherwise, right-justify
/// @return Number of characters written
static s32
fmt_format_uint(fmt_putc_fn putc, void * context, u64 value, u32 base, bool upper, u32 width, char pad, bool left);

/// @brief Format a signed integer with the given options, outputting via putc.
/// @param putc Output callback function
/// @param context Opaque context passed to putc
/// @param value Signed integer value to format
/// @param width Minimum field width (pad with 'pad' character if necessary)
/// @param pad Padding character (e.g., ' ' or '0')
/// @param left If true, left-justify the output; otherwise, right-justify
/// @return Number of characters written
static s32 fmt_format_int(fmt_putc_fn putc, void * context, s64 value, u32 width, char pad, bool left);

/// @brief Format a string with the given options, outputting via putc.
/// @param putc Output callback function
/// @param context Opaque context passed to putc
/// @param str String to format (if NULL, outputs "(null)")
/// @param width Minimum field width (pad with spaces if necessary)
/// @param left If true, left-justify the output; otherwise, right-justify
/// @return Number of characters written
static s32 fmt_format_string(fmt_putc_fn putc, void * context, char const * str, u32 width, bool left);

/// @brief Core implementation of vfmt_to, which parses the format string and handles all formatting logic.
/// @param putc Output callback function
/// @param context Opaque context passed to putc
/// @param format Format string (printf-style)
/// @param ap va_list of arguments (must be initialized by caller via va_start)
/// @return Number of characters written
static s32 vfmt_impl(fmt_putc_fn putc, void * context, char const * format, va_list ap);

// Buffer sink for snprintf/sprintf
typedef struct {
    char * out_buffer;
    u64 position;
    u64 max;
} buffer_ctx_t;

/// @brief Buffer sink callback for snprintf/sprintf.
/// @param c Character to write
/// @param context Pointer to buffer_ctx_t
static void buffer_putc(char c, void * context);

s32 fmt_to(fmt_putc_fn putc, void * context, char const * format, ...)
{
    va_list ap;
    va_start(ap, format);
    s32 ret = vfmt_impl(putc, context, format, ap);
    va_end(ap);
    return ret;
}

s32 vfmt_to(fmt_putc_fn putc, void * context, char const * format, va_list ap)
{
    return vfmt_impl(putc, context, format, ap);
}

s32 snprintf(char * out_buffer, u64 buflen, char const * format, ...)
{
    va_list ap;
    va_start(ap, format);
    buffer_ctx_t bctx = {.out_buffer = out_buffer, .position = 0, .max = buflen};
    s32 ret = vfmt_impl(buffer_putc, &bctx, format, ap);
    va_end(ap);
    if (buflen > 0) {
        out_buffer[(bctx.position < buflen) ? bctx.position : (buflen - 1)] = '\0';
    }
    return ret;
}

s32 sprintf(char * out_buffer, char const * format, ...)
{
    va_list ap;
    va_start(ap, format);
    // Use a very large max for 'infinite' buffer
    buffer_ctx_t bctx = {.out_buffer = out_buffer, .position = 0, .max = (u64) -1};
    s32 ret = vfmt_impl(buffer_putc, &bctx, format, ap);
    va_end(ap);
    out_buffer[bctx.position] = '\0';
    return ret;
}

static void buffer_putc(char c, void * context)
{
    buffer_ctx_t * b = (buffer_ctx_t *) context;
    // Guard against b->max == 0: subtracting 1 from a u64 zero would underflow.
    if (b->max > 0 && b->position < b->max - 1) {
        b->out_buffer[b->position] = c;
    }
    b->position++;
}

static s32
fmt_format_uint(fmt_putc_fn putc, void * context, u64 value, u32 base, bool upper, u32 width, char pad, bool left)
{
    char digits[20];
    s32 position = 0;
    char const * alphabet = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    u64 tmp = value;
    s32 length = 0;

    // Special case: zero
    if (tmp == 0) {
        digits[position++] = '0';
    } else {
        while (tmp) {
            digits[position++] = alphabet[tmp % base];
            tmp /= base;
        }
    }
    length = position;

    s32 padlen = ((s32) width > length) ? ((s32) width - length) : 0;
    if (!left) {
        for (s32 i = 0; i < padlen; ++i) {
            putc(pad, context);
        }
    }
    for (s32 i = length - 1; i >= 0; --i) {
        putc(digits[i], context);
    }
    if (left) {
        for (s32 i = 0; i < padlen; ++i) {
            putc(' ', context);
        }
    }
    return (length + padlen);
}

static s32 fmt_format_int(fmt_putc_fn putc, void * context, s64 value, u32 width, char pad, bool left)
{
    s32 count = 0;
    u64 absval = (value < 0) ? -(u64) value : (u64) value;
    if (value < 0) {
        putc('-', context);
        ++count;
        if (width > 0) {
            --width;
        }
    }
    count += fmt_format_uint(putc, context, absval, 10, false, width, pad, left);
    return count;
}

static s32 fmt_format_string(fmt_putc_fn putc, void * context, char const * str, u32 width, bool left)
{
    s32 length = 0;
    char const * s = str ? str : "(null)";
    while (s[length]) {
        ++length;
    }
    s32 padlen = ((s32) width > length) ? ((s32) width - length) : 0;
    if (!left) {
        for (s32 i = 0; i < padlen; ++i) {
            putc(' ', context);
        }
    }
    for (s32 i = 0; i < length; ++i) {
        putc(s[i], context);
    }
    if (left) {
        for (s32 i = 0; i < padlen; ++i) {
            putc(' ', context);
        }
    }
    return length + padlen;
}

// NOLINTBEGIN(readability-function-size,readability-function-cognitive-complexity,clang-analyzer-valist.Uninitialized)
// printf-style formatter is inherently complex; splitting it would harm readability.
// clang-analyzer-valist.Uninitialized: false positive — ap is always initialized via va_start by every caller.
static s32 vfmt_impl(fmt_putc_fn putc, void * context, char const * format, va_list ap)
{
    s32 count = 0;
    char ch;
    while ((ch = *format++)) {
        if (ch != '%') {
            putc(ch, context);
            ++count;
            continue;
        }
        // Parse flags
        bool left = false;
        bool upper = false;
        char pad = ' ';
        if (*format == '-') {
            left = true;
            ++format;
        }
        if (*format == '0') {
            pad = '0';
            ++format;
        }
        // Parse width
        u32 width = 0;
        while (*format >= '0' && *format <= '9') {
            width = (width * 10) + (u32) (*format++ - '0');
        }
        // Parse length (l, ll)
        u32 longmod = 0;
        if (*format == 'l') {
            ++format;
            longmod = 1;
            if (*format == 'l') {
                ++format;
                longmod = 2;
            }
        }
        // Parse specifier
        ch = *format++;
        switch (ch) {
        case 'd':
        case 'i':
            {
                s64 val = (longmod == 2) ? va_arg(ap, s64) : (longmod == 1) ? va_arg(ap, long) : va_arg(ap, int);
                count += fmt_format_int(putc, context, val, width, pad, left);
                break;
            }
        case 'u':
            {
                u64 val = (longmod == 2)   ? va_arg(ap, u64)
                          : (longmod == 1) ? va_arg(ap, unsigned long)
                                           : va_arg(ap, unsigned int);
                count += fmt_format_uint(putc, context, val, 10, false, width, pad, left);
                break;
            }
        case 'x':
        case 'X':
            {
                upper = (ch == 'X');
                u64 val = (longmod == 2)   ? va_arg(ap, u64)
                          : (longmod == 1) ? va_arg(ap, unsigned long)
                                           : va_arg(ap, unsigned int);
                count += fmt_format_uint(putc, context, val, 16, upper, width, pad, left);
                break;
            }
        case 'p':
            {
                putc('0', context);
                putc('x', context);
                count += 2;
                u64 val = (u64) (uintptr_t) va_arg(ap, void *);
                count += fmt_format_uint(putc, context, val, 16, false, 16, '0', false);
                break;
            }
        case 's':
            {
                char const * str = va_arg(ap, char const *);
                count += fmt_format_string(putc, context, str, width, left);
                break;
            }
        case 'c':
            {
                char c = (char) va_arg(ap, int);
                putc(c, context);
                ++count;
                break;
            }
        case '%':
            {
                putc('%', context);
                ++count;
                break;
            }
        default:
            putc('%', context);
            putc(ch, context);
            count += 2;
            break;
        }
    }
    return count;
}
// NOLINTEND(readability-function-size,readability-function-cognitive-complexity,clang-analyzer-valist.Uninitialized)