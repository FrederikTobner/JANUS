#ifndef BOOT_PROTOCOLS_H
#define BOOT_PROTOCOLS_H

#include <tinyos/types.h>

enum boot_protocol {
    BOOT_PROTOCOL_UNKNOWN   = 0,
    BOOT_PROTOCOL_MULTIBOOT2 = 1,
};

typedef u32 boot_protocol_t;

#endif /* BOOT_PROTOCOLS_H */
