#include <lib/memory.h>

/**
 * Memory manipulation operations implementation
 * 
 * Basic implementations - can be optimized later with
 * architecture-specific SIMD or specialized instructions.
 */

void * memcpy(void * dest, void const * src, size_t n)
{
    uint8_t * d = (uint8_t *) dest;
    uint8_t const * s = (uint8_t const *) src;
    
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
    
    return dest;
}

void * memmove(void * dest, void const * src, size_t n)
{
    uint8_t * d = (uint8_t *) dest;
    uint8_t const * s = (uint8_t const *) src;
    
    if (d < s) {
        // Copy forward
        for (size_t i = 0; i < n; i++) {
            d[i] = s[i];
        }
    } else {
        // Copy backward
        for (size_t i = n; i > 0; i--) {
            d[i - 1] = s[i - 1];
        }
    }
    
    return dest;
}

void * memset(void * dest, int c, size_t n)
{
    uint8_t * d = (uint8_t *) dest;
    uint8_t value = (uint8_t) c;
    
    for (size_t i = 0; i < n; i++) {
        d[i] = value;
    }
    
    return dest;
}

int memcmp(void const * s1, void const * s2, size_t n)
{
    uint8_t const * p1 = (uint8_t const *) s1;
    uint8_t const * p2 = (uint8_t const *) s2;
    
    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] - p2[i];
        }
    }
    
    return 0;
}

void * memzero(void * dest, size_t n)
{
    return memset(dest, 0, n);
}
