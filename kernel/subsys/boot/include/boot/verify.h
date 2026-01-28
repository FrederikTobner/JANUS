#ifndef BOOT_VERIFY_H
#define BOOT_VERIFY_H

#include <janus/types.h>

/**
 * Verify that the boot handoff is valid for the currently supported protocols.
 *
 * Returns 0 on success, non-zero on failure.
 */
error_t boot_verify_handoff(u64 loader_magic, void * info);

#endif /* BOOT_VERIFY_H */
