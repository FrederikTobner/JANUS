/*****************************************************************************
 * Copyright (C) 2025 by Frederik Tobner                                     *
 *                                                                           *
 * This file is part of JANUS                                                *
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

#ifndef JANUS_ATTRIBUTES_H
#define JANUS_ATTRIBUTES_H

/// Compiler-specific attributes and macros for JANUS
///
/// Provides portable compiler attributes for Clang and GCC.

// Function and type attributes

// Indicates the function never returns to its caller (e.g. halt loops,
// abort). Allows the compiler to omit unreachable code after the call and
// to warn if the function *does* contain a reachable return.
#define __noreturn               __attribute__((noreturn))

// Eliminates internal padding and trailing padding from a struct or union,
// storing members at the minimum byte offset.  Accesses to misaligned
// fields are emitted as byte-by-byte loads/stores on targets that do not
// support unaligned access.
#define __packed                 __attribute__((packed))

// Sets the minimum alignment (in bytes) of a variable, struct field, or
// type.  The compiler may place the object at a *stricter* alignment, but
// never a weaker one.
#define __aligned(x)             __attribute__((aligned(x)))

// Suppresses "unused variable/parameter/function" warnings.  The entity
// is still emitted; only the diagnostic is silenced.
#define __unused                 __attribute__((unused))

// Tells the compiler and linker to retain the symbol even if it appears
// unreferenced.  Prevents removal by --gc-sections and LTO.  Typical
// use: variables or functions placed in special linker sections.
#define __used                   __attribute__((used))

// Places the variable or function in the named ELF section instead of the
// default .text / .data / .bss.  The section must be defined by the
// linker script or created implicitly.
#define __section(s)             __attribute__((section(s)))

// Emits a weak symbol.  A weak definition is overridden by any non-weak
// (strong) definition at link time.  If no strong definition exists the
// weak one is used.  Useful for providing defaults.
#define __weak                   __attribute__((weak))

// Forces the compiler to inline the function at every call site, even at
// -O0.  A compilation error is emitted if the function cannot be inlined
// (e.g. address taken and used indirectly).
#define __always_inline          __attribute__((always_inline)) inline

// Prevents the compiler from inlining the function regardless of
// optimisation level or apparent profitability.
#define __noinline               __attribute__((noinline))

// Declares a function as pure: its return value depends only on its
// arguments and on global memory, and the function has no observable side
// effects other than its return value.  The compiler may hoist or
// eliminate redundant calls with identical arguments when the memory they
// read has not changed.  Reads through pointers are permitted; writes are
// not.
#define __pure                   __attribute__((pure))

// Stricter than __pure.  The return value depends *only* on the arguments
// — the function must not read global memory, dereference pointers, or
// call non-const functions.  The compiler may freely reorder, merge, or
// eliminate calls with the same argument values.
#define __const                  __attribute__((const))

// Hints that the function is unlikely to be called (error paths, one-time
// initialisation).  The compiler may move its code to a distant text
// section, reduce inlining aggressiveness, and optimise branch prediction
// for the non-cold path.
#define __cold                   __attribute__((cold))

// Hints that the function is called frequently.  The compiler may place
// it in a hot text section, increase inlining aggressiveness, and optimise
// for the hot path in branch prediction.
#define __hot                    __attribute__((hot))

// Likely/unlikely for branch prediction
#define LIKELY(x)                __builtin_expect(!!(x), 1)
#define UNLIKELY(x)              __builtin_expect(!!(x), 0)

// Static assertion
#define STATIC_ASSERT(expr, msg) _Static_assert(expr, msg)

// Compile-time check — triggers a compile error when condition is true.
// Mirrors Linux's BUILD_BUG_ON for familiarity.
#define BUILD_BUG_ON(condition)  STATIC_ASSERT(!(condition), "BUILD_BUG_ON triggered")

// Array size
#define ARRAY_SIZE(arr)          (sizeof(arr) / sizeof((arr)[0]))

// Offset of member in struct (for CONTAINER_OF)
#define __offsetof(type, member) __builtin_offsetof(type, member)

// Container of macro.
// Uses a compound-statement (GCC/Clang extension) so that __typeof__ can
// verify at compile time that ptr has the same type as &container->member,
// catching accidental pointer-type mismatches early.
#define CONTAINER_OF(ptr, type, member)                          \
    ({                                                           \
        __typeof__(((type *) 0)->member) const * __mptr = (ptr); \
        (type *) ((char *) __mptr - __offsetof(type, member));   \
    })

#endif // JANUS_ATTRIBUTES_H
