#ifndef BOOT_VERIFY_H
#define BOOT_VERIFY_H

#include <boot/handoff.h>

/**
 * Verify that the boot handoff is valid for the currently supported protocols.
 *
 * Returns 0 on success, non-zero on failure.
 */
int boot_verify_handoff(struct boot_handoff const *handoff);

#endif /* BOOT_VERIFY_H */
