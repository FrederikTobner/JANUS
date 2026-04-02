/*
 * @file output.c
 * @brief Kernel formatted output: number/string formatting helpers (internal)
 * @copyright Copyright (C) 2026 Frederik, TinyOS contributors
 * @license GNU Affero General Public License v3.0 or later
 */
#include <fmt/output.h>
#include <janus/types.h>
#include <janus/va_arg.h>

// Internal helpers for formatting numbers and strings

static s32
fmt_format_uint(fmt_putc_fn putc, void * ctx, u64 value, u32 base, bool upper, u32 width, char pad, bool left)
{
    char digits[20];
    s32 pos = 0;
    char const * alphabet = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    u64 tmp = value;
    s32 len = 0;

    // Special case: zero
    if (tmp == 0) {
        digits[pos++] = '0';
    } else {
        while (tmp) {
            digits[pos++] = alphabet[tmp % base];
            tmp /= base;
        }
    }
    len = pos;

    s32 padlen = ((s32) width > len) ? ((s32) width - len) : 0;
    if (!left) {
        for (s32 i = 0; i < padlen; ++i) {
            putc(pad, ctx);
        }
    }
    for (s32 i = len - 1; i >= 0; --i) {
        putc(digits[i], ctx);
    }
    if (left) {
        for (s32 i = 0; i < padlen; ++i) {
            putc(' ', ctx);
        }
    }
    return (len + padlen);
}

static s32 fmt_format_int(fmt_putc_fn putc, void * ctx, s64 value, u32 width, char pad, bool left)
{
    s32 count = 0;
    u64 absval = (value < 0) ? -(u64) value : (u64) value;
    if (value < 0) {
        putc('-', ctx);
        ++count;
        if (width > 0) {
            --width;
        }
    }
    count += fmt_format_uint(putc, ctx, absval, 10, false, width, pad, left);
    return count;
}

static s32 fmt_format_string(fmt_putc_fn putc, void * ctx, char const * str, u32 width, bool left)
{
    s32 len = 0;
    char const * s = str ? str : "(null)";
    while (s[len]) {
        ++len;
    }
    s32 padlen = ((s32) width > len) ? ((s32) width - len) : 0;
    if (!left) {
        for (s32 i = 0; i < padlen; ++i) {
            putc(' ', ctx);
        }
    }
    for (s32 i = 0; i < len; ++i) {
        putc(s[i], ctx);
    }
    if (left) {
        for (s32 i = 0; i < padlen; ++i) {
            putc(' ', ctx);
        }
    }
    return len + padlen;
}

static s32 vfmt_impl(fmt_putc_fn putc, void * ctx, char const * fmt, va_list ap)
{
    s32 count = 0;
    char ch;
    while ((ch = *fmt++)) {
        if (ch != '%') {
            putc(ch, ctx);
            ++count;
            continue;
        }
        // Parse flags
        bool left = false;
        bool upper = false;
        char pad = ' ';
        if (*fmt == '-') {
            left = true;
            ++fmt;
        }
        if (*fmt == '0') {
            pad = '0';
            ++fmt;
        }
        // Parse width
        u32 width = 0;
        while (*fmt >= '0' && *fmt <= '9') {
            width = width * 10 + (u32) (*fmt++ - '0');
        }
        // Parse length (l, ll)
        u32 longmod = 0;
        if (*fmt == 'l') {
            ++fmt;
            longmod = 1;
            if (*fmt == 'l') {
                ++fmt;
                longmod = 2;
            }
        }
        // Parse specifier
        ch = *fmt++;
        switch (ch) {
        case 'd':
        case 'i':
            {
                s64 val = (longmod == 2) ? va_arg(ap, s64) : (longmod == 1) ? va_arg(ap, long) : va_arg(ap, int);
                count += fmt_format_int(putc, ctx, val, width, pad, left);
                break;
            }
        case 'u':
            {
                u64 val = (longmod == 2)   ? va_arg(ap, u64)
                          : (longmod == 1) ? va_arg(ap, unsigned long)
                                           : va_arg(ap, unsigned int);
                count += fmt_format_uint(putc, ctx, val, 10, false, width, pad, left);
                break;
            }
        case 'x':
        case 'X':
            {
                upper = (ch == 'X') ? true : false;
                u64 val = (longmod == 2)   ? va_arg(ap, u64)
                          : (longmod == 1) ? va_arg(ap, unsigned long)
                                           : va_arg(ap, unsigned int);
                count += fmt_format_uint(putc, ctx, val, 16, upper, width, pad, left);
                break;
            }
        case 'p':
            {
                putc('0', ctx);
                putc('x', ctx);
                count += 2;
                u64 val = (u64) (uintptr_t) va_arg(ap, void *);
                count += fmt_format_uint(putc, ctx, val, 16, false, 16, '0', false);
                break;
            }
        case 's':
            {
                char const * str = va_arg(ap, char const *);
                count += fmt_format_string(putc, ctx, str, width, left);
                break;
            }
        case 'c':
            {
                char c = (char) va_arg(ap, int);
                putc(c, ctx);
                ++count;
                break;
            }
        case '%':
            {
                putc('%', ctx);
                ++count;
                break;
            }
        default:
            putc('%', ctx);
            putc(ch, ctx);
            count += 2;
            break;
        }
    }
    return count;
}

s32 fmt_to(fmt_putc_fn putc, void * ctx, char const * fmtstr, ...)
{
    va_list ap;
    va_start(ap, fmtstr);
    s32 ret = vfmt_impl(putc, ctx, fmtstr, ap);
    va_end(ap);
    return ret;
}

s32 vfmt_to(fmt_putc_fn putc, void * ctx, char const * fmtstr, va_list ap)
{
    return vfmt_impl(putc, ctx, fmtstr, ap);
}

// Buffer sink for snprintf/sprintf
typedef struct {
    char * buf;
    u64 pos;
    u64 max;
} buffer_ctx_t;

static void buffer_putc(char c, void * ctx)
{
    buffer_ctx_t * b = (buffer_ctx_t *) ctx;
    if (b->pos < b->max - 1) {
        b->buf[b->pos] = c;
    }
    b->pos++;
}

s32 snprintf(char * buf, u64 buflen, char const * fmtstr, ...)
{
    va_list ap;
    va_start(ap, fmtstr);
    buffer_ctx_t bctx = {.buf = buf, .pos = 0, .max = buflen};
    s32 ret = vfmt_impl(buffer_putc, &bctx, fmtstr, ap);
    va_end(ap);
    if (buflen > 0) {
        buf[(bctx.pos < buflen) ? bctx.pos : (buflen - 1)] = '\0';
    }
    return ret;
}

s32 sprintf(char * buf, char const * fmtstr, ...)
{
    va_list ap;
    va_start(ap, fmtstr);
    // Use a very large max for 'infinite' buffer
    buffer_ctx_t bctx = {.buf = buf, .pos = 0, .max = (u64) -1};
    s32 ret = vfmt_impl(buffer_putc, &bctx, fmtstr, ap);
    va_end(ap);
    buf[bctx.pos] = '\0';
    return ret;
}