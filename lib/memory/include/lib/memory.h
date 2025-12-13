#ifndef LIB_MEMORY_H
#define LIB_MEMORY_H

#include <lib/types.h>

/**
 * Memory manipulation operations for TinyOS
 * 
 * Provides low-level memory operations without relying on libc.
 * Functions operate on raw memory regions.
 */

/**
 * Copy memory from source to destination
 * 
 * @param dest Destination pointer
 * @param src Source pointer
 * @param n Number of bytes to copy
 * @return dest pointer
 * 
 * @note Regions must not overlap. Use memmove for overlapping regions.
 */
void * memcpy(void * dest, void const * src, size_t n);

/**
 * Copy memory with support for overlapping regions
 * 
 * @param dest Destination pointer
 * @param src Source pointer
 * @param n Number of bytes to copy
 * @return dest pointer
 */
void * memmove(void * dest, void const * src, size_t n);

/**
 * Set memory to a specific byte value
 * 
 * @param dest Destination pointer
 * @param c Byte value to set (converted to unsigned char)
 * @param n Number of bytes to set
 * @return dest pointer
 */
void * memset(void * dest, int c, size_t n);

/**
 * Compare two memory regions
 * 
 * @param s1 First memory region
 * @param s2 Second memory region
 * @param n Number of bytes to compare
 * @return 0 if equal, <0 if s1 < s2, >0 if s1 > s2
 */
int memcmp(void const * s1, void const * s2, size_t n);

/**
 * Zero-fill memory region
 * 
 * @param dest Destination pointer
 * @param n Number of bytes to zero
 * @return dest pointer
 * 
 * @note This is memset(dest, 0, n) but explicit about zeroing
 */
void * memzero(void * dest, size_t n);

#endif // LIB_MEMORY_H
