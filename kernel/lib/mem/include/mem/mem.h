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

/// @file mem.h
/// @brief Interface for basic memory manipulation functions.

#ifndef JANUS_MEM_H
#define JANUS_MEM_H

#include <janus/types.h>

/// @brief Fill a memory region with a single byte value.
/// @param dest Destination pointer
/// @param value Byte value to store
/// @param n Number of bytes to write
/// @return dest
void * mem_set(void * dest, u8 value, u64 n);

/// @brief Copy bytes from src to dest. Source and destination must not overlap.
/// @param dest Destination pointer
/// @param src Source pointer
/// @param n Number of bytes to copy
/// @return dest
void * mem_copy(void * dest, void const * src, u64 n);

/// @brief Copy bytes from src to dest. Handles overlapping regions.
/// @param dest Destination pointer
/// @param src Source pointer
/// @param n Number of bytes to move
/// @return dest
void * mem_move(void * dest, void const * src, u64 n);

/// @brief Compare two byte ranges lexicographically.
/// @param a First pointer
/// @param b Second pointer
/// @param n Number of bytes to compare
/// @return Negative if a < b, zero if equal, positive if a > b
s32 mem_compare(void const * a, void const * b, u64 n);

#endif // JANUS_MEM_H
