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

#ifndef JANUS_TYPES_H
#define JANUS_TYPES_H

/**
 * @file types.h
 * @brief Global kernel type definitions
 *
 * This header defines kernel-specific types used throughout JANUS.
 * These types provide semantic meaning beyond basic integer sizes.
 */

#include <uapi/types.h>

// Physical and virtual memory addresses
typedef __u64 phys_addr_t; ///< Physical memory address
typedef __u64 virt_addr_t; ///< Virtual memory address

// Page frame number (physical page identifier)
typedef __u64 pfn_t; ///< Page frame number

// Process and thread identifiers
typedef __s32 pid_t; ///< Process ID
typedef __s32 tid_t; ///< Thread ID

// Device number
typedef __u32 dev_t; ///< Device number

// Generic error type for kernel operations
typedef __s32 error_t; ///< Error code (negative = error, 0 = success, positive = info)

// Kernl shorthands for integers
typedef __s8 s8;
typedef __u8 u8;
typedef __s16 s16;
typedef __u16 u16;
typedef __s32 s32;
typedef __u32 u32;
typedef __s64 s64;
typedef __u64 u64;

// Kenrnel shorthands for boolean
#define bool                          __bool
#define true                          1
#define false                         0
#define __bool_true_false_are_defined 1

#endif /* JANUS_TYPES_H */
