#ifndef LIB_COMPILER_H
#define LIB_COMPILER_H

/**
 * Compiler-specific attributes and macros for TinyOS
 * 
 * Provides portable compiler attributes for Clang and GCC.
 */

// Compiler detection
#if defined(__clang__)
    #define COMPILER_CLANG 1
#elif defined(__GNUC__)
    #define COMPILER_GCC 1
#else
    #error "Unsupported compiler - TinyOS requires Clang or GCC"
#endif

// Function attributes
#define __noreturn      __attribute__((noreturn))
#define __packed        __attribute__((packed))
#define __aligned(x)    __attribute__((aligned(x)))
#define __unused        __attribute__((unused))
#define __used          __attribute__((used))
#define __section(s)    __attribute__((section(s)))
#define __weak          __attribute__((weak))
#define __always_inline __attribute__((always_inline)) inline
#define __noinline      __attribute__((noinline))
#define __pure          __attribute__((pure))
#define __const         __attribute__((const))
#define __cold          __attribute__((cold))
#define __hot           __attribute__((hot))

// Likely/unlikely for branch prediction
#define LIKELY(x)       __builtin_expect(!!(x), 1)
#define UNLIKELY(x)     __builtin_expect(!!(x), 0)

// Barrier for compiler optimization
#define BARRIER()       __asm__ __volatile__("" ::: "memory")

// Static assertion
#define STATIC_ASSERT(expr, msg) _Static_assert(expr, msg)

// Array size
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

// Container of macro
#define CONTAINER_OF(ptr, type, member) \
    ((type *) ((char *) (ptr) - offsetof(type, member)))

// Min/max macros
#define MIN(a, b) \
    ({ __typeof__(a) _a = (a); \
       __typeof__(b) _b = (b); \
       _a < _b ? _a : _b; })

#define MAX(a, b) \
    ({ __typeof__(a) _a = (a); \
       __typeof__(b) _b = (b); \
       _a > _b ? _a : _b; })

#endif // LIB_COMPILER_H
