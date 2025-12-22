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

#ifndef UAPI_TYPES_H
#define UAPI_TYPES_H

/**
 * @file int-ll64.h
 * @brief 64-bit integer types (TinyOS freestanding implementation)
 */


/**
 * @brief 64-bit integer types (TinyOS freestanding implementation)
 *
 * This is TinyOS's implementation of the C standard stdint.h header.
 * In freestanding mode, we provide our own using compiler builtins.
 */

/* Unsigned integer types */
typedef __UINT8_TYPE__   __u8;
typedef __UINT16_TYPE__  __u16;
typedef __UINT32_TYPE__  __u32;
typedef __UINT64_TYPE__  __u64;

/* Signed integer types */
typedef __INT8_TYPE__    __s8;
typedef __INT16_TYPE__   __s16;
typedef __INT32_TYPE__   __s32;
typedef __INT64_TYPE__   __s64;

// Define boolean type
#define __bool _Bool;

#endif /* UAPI_TYPES_H */
