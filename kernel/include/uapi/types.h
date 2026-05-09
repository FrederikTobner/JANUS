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

#ifndef UAPI_TYPES_H
#define UAPI_TYPES_H

/// @file int-ll64.h
/// @brief 64-bit integer types (JANUS freestanding implementation)

/// @brief 64-bit integer types (JANUS freestanding implementation)
///
/// This is JANUS's implementation of the C standard stdint.h header.
/// In freestanding mode, we provide our own using compiler builtins.

typedef __UINT8_TYPE__ __u8;
typedef __INT8_TYPE__ __s8;
typedef __UINT16_TYPE__ __u16;
typedef __INT16_TYPE__ __s16;
typedef __UINT32_TYPE__ __u32;
typedef __INT32_TYPE__ __s32;

// Forcing 8-byte alignment for 64-bit types to avoid issues when receiving a struct in a 32-bit userspace process,
// which may not have 64-bit alignment. This can occur when we do a system call that either expects a struct with 64-bit
// fields or returns a struct with 64-bit fields, and the userspace process is compiled as 32-bit.
typedef __UINT64_TYPE__ __u64 __attribute__((aligned(8)));
typedef __INT64_TYPE__ __s64 __attribute__((aligned(8)));

_Static_assert(sizeof(__u8) == 1, "__u8 must be 1 byte");
_Static_assert(sizeof(__s8) == 1, "__s8 must be 1 byte");
_Static_assert(sizeof(__u16) == 2, "__u16 must be 2 bytes");
_Static_assert(sizeof(__s16) == 2, "__s16 must be 2 bytes");
_Static_assert(sizeof(__u32) == 4, "__u32 must be 4 bytes");
_Static_assert(sizeof(__s32) == 4, "__s32 must be 4 bytes");
_Static_assert(sizeof(__u64) == 8, "__u64 must be 8 bytes");
_Static_assert(sizeof(__s64) == 8, "__s64 must be 8 bytes");

#endif /* UAPI_TYPES_H */
