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

#ifndef LIB_MEMORY_H
#define LIB_MEMORY_H

/**
 * @file memory.h
 * @brief Memory manipulation operations for JANUS
 *
 * Provides freestanding implementations of standard memory functions
 * (memcpy, memset, memmove, memcmp, memzero) for kernel use.
 *
 * These are basic implementations that can be optimized later with
 * architecture-specific SIMD or specialized instructions.
 */

#include <janus/types.h>
#include <stddef.h>

/**
 * @brief Copy memory from source to destination
 *
 * Copies n bytes from src to dest. Regions must not overlap.
 * For overlapping regions, use memmove().
 *
 * @param dest Destination pointer
 * @param src Source pointer
 * @param n Number of bytes to copy
 * @return dest pointer
 */
void * memcpy(void * dest, void const * src, size_t n);

/**
 * @brief Copy memory with overlap handling
 *
 * Copies n bytes from src to dest, correctly handling overlapping regions.
 *
 * @param dest Destination pointer
 * @param src Source pointer
 * @param n Number of bytes to copy
 * @return dest pointer
 */
void * memmove(void * dest, void const * src, size_t n);

/**
 * @brief Fill memory with a constant byte
 *
 * Sets n bytes at dest to the value c (converted to unsigned char).
 *
 * @param dest Destination pointer
 * @param c Value to set (only lowest byte used)
 * @param n Number of bytes to set
 * @return dest pointer
 */
void * memset(void * dest, int c, size_t n);

/**
 * @brief Compare memory regions
 *
 * Compares n bytes at s1 and s2.
 *
 * @param s1 First memory region
 * @param s2 Second memory region
 * @param n Number of bytes to compare
 * @return 0 if equal, <0 if s1 < s2, >0 if s1 > s2
 */
int memcmp(void const * s1, void const * s2, size_t n);

/**
 * @brief Zero-fill memory
 *
 * Sets n bytes at dest to zero. Convenience wrapper around memset.
 *
 * @param dest Destination pointer
 * @param n Number of bytes to zero
 * @return dest pointer
 */
void * memzero(void * dest, size_t n);

#endif // LIB_MEMORY_H