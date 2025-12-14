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

#ifndef TINYOS_TYPES_H
#define TINYOS_TYPES_H

/**
 * @file types.h
 * @brief Global kernel type definitions
 * 
 * This header defines kernel-specific types used throughout TinyOS.
 * These are high-level kernel abstractions built on top of lib/types.h.
 */

#include <lib/types.h>

// Physical and virtual memory addresses
typedef uint64_t phys_addr_t;  ///< Physical memory address
typedef uint64_t virt_addr_t;  ///< Virtual memory address

// Page frame number (physical page identifier)
typedef uint64_t pfn_t;        ///< Page frame number

// Process and thread identifiers
typedef int32_t pid_t;         ///< Process ID
typedef int32_t tid_t;         ///< Thread ID

// Device number
typedef uint32_t dev_t;        ///< Device number

// Generic error type for kernel operations
typedef int32_t error_t;       ///< Error code (negative = error, 0 = success, positive = info)

#endif /* TINYOS_TYPES_H */
