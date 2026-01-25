#ifndef BOOT_HANDOFF_H
#define BOOT_HANDOFF_H

#include <boot/protocols.h>
#include <janus/types.h>

/**
 * Minimal boot handoff structure.
 *
 * The boot layer constructs this and passes it to the kernel entry point.
 * It is intentionally small: just enough to validate that we were booted
 * via the expected protocol and that the boot info pointer is present.
 */
struct boot_handoff {
    boot_protocol_t protocol;
    u32 reserved;

    u64 loader_magic;
    void * loader_info;
};

#endif /* BOOT_HANDOFF_H */
