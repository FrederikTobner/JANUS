/*****************************************************************************
 * Copyright (C) 2025 by Frederik Tobner                                     *
 *                                                                           *
 * This file is part of TinyOs.                                              *
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

#ifndef _STDINT_H
#define _STDINT_H

/**
 * @file stdint.h
 * @brief Fixed-width integer types (TinyOS freestanding implementation)
 * 
 * This is TinyOS's implementation of the C standard stdint.h header.
 * In freestanding mode, we provide our own using compiler builtins.
 */

/* Unsigned integer types */
typedef __UINT8_TYPE__   uint8_t;
typedef __UINT16_TYPE__  uint16_t;
typedef __UINT32_TYPE__  uint32_t;
typedef __UINT64_TYPE__  uint64_t;

/* Signed integer types */
typedef __INT8_TYPE__    int8_t;
typedef __INT16_TYPE__   int16_t;
typedef __INT32_TYPE__   int32_t;
typedef __INT64_TYPE__   int64_t;

/* Pointer-sized integers */
typedef __INTPTR_TYPE__  intptr_t;
typedef __UINTPTR_TYPE__ uintptr_t;

/* Limits for unsigned types */
#define UINT8_MAX   0xFF
#define UINT16_MAX  0xFFFF
#define UINT32_MAX  0xFFFFFFFFU
#define UINT64_MAX  0xFFFFFFFFFFFFFFFFULL

/* Limits for signed types */
#define INT8_MIN    (-128)
#define INT8_MAX    127
#define INT16_MIN   (-32768)
#define INT16_MAX   32767
#define INT32_MIN   (-2147483648)
#define INT32_MAX   2147483647
#define INT64_MIN   (-9223372036854775807LL - 1)
#define INT64_MAX   9223372036854775807LL

/* Pointer-sized limits */
#define INTPTR_MIN  INT64_MIN
#define INTPTR_MAX  INT64_MAX
#define UINTPTR_MAX UINT64_MAX

#endif /* _STDINT_H */
