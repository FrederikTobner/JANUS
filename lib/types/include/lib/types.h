#ifndef LIB_TYPES_H
#define LIB_TYPES_H

/**
 * Basic type definitions for TinyOS
 * 
 * Provides fixed-width integer types, size types, and boolean type
 * for freestanding kernel environment.
 */

// Fixed-width integer types
typedef signed char        int8_t;
typedef unsigned char      uint8_t;
typedef signed short       int16_t;
typedef unsigned short     uint16_t;
typedef signed int         int32_t;
typedef unsigned int       uint32_t;
typedef signed long long   int64_t;
typedef unsigned long long uint64_t;

// Pointer-sized integers
typedef long          intptr_t;
typedef unsigned long uintptr_t;

// Size types
typedef unsigned long size_t;
typedef long          ssize_t;
typedef long          ptrdiff_t;

// Boolean type
typedef _Bool bool;
#define true  1
#define false 0

// NULL pointer
#ifndef NULL
#define NULL ((void *) 0)
#endif

// Integer limits
#define INT8_MIN   (-128)
#define INT8_MAX   127
#define UINT8_MAX  255U

#define INT16_MIN  (-32768)
#define INT16_MAX  32767
#define UINT16_MAX 65535U

#define INT32_MIN  (-2147483648)
#define INT32_MAX  2147483647
#define UINT32_MAX 4294967295U

#define INT64_MIN  (-9223372036854775808LL)
#define INT64_MAX  9223372036854775807LL
#define UINT64_MAX 18446744073709551615ULL

#define SIZE_MAX   UINT64_MAX
#define SSIZE_MAX  INT64_MAX

// Common macros
#define offsetof(type, member) __builtin_offsetof(type, member)

#endif // LIB_TYPES_H
