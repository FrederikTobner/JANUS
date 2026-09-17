/*****************************************************************************
 * Copyright (C) 2026 by Frederik Tobner                                     *
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

/// @file mem.c
/// @brief Implementation of basic memory manipulation functions.

#include <mem/mem.h>

void * mem_set(void * dest, u8 value, u64 n)
{
    u8 * d = (u8 *) dest;
    for (u64 i = 0; i < n; ++i) {
        d[i] = value;
    }
    return dest;
}

void * mem_copy(void * dest, void const * src, u64 n)
{
    u8 * d = (u8 *) dest;
    u8 const * s = (u8 const *) src;
    for (u64 i = 0; i < n; ++i) {
        d[i] = s[i];
    }
    return dest;
}

void * mem_move(void * dest, void const * src, u64 n)
{
    u8 * d = (u8 *) dest;
    u8 const * s = (u8 const *) src;

    if (d == s || n == 0) {
        return dest;
    }

    if (d < s || d >= (s + n)) {
        for (u64 i = 0; i < n; ++i) {
            d[i] = s[i];
        }
    } else {
        for (u64 i = n; i > 0; --i) {
            d[i - 1] = s[i - 1];
        }
    }

    return dest;
}

s32 mem_compare(void const * a, void const * b, u64 n)
{
    u8 const * aa = (u8 const *) a;
    u8 const * bb = (u8 const *) b;

    for (u64 i = 0; i < n; ++i) {
        if (aa[i] != bb[i]) {
            return (aa[i] < bb[i]) ? -1 : 1;
        }
    }
    return 0;
}
