#include <boot/multiboot2.h>
#include <boot/protocols.h>
#include <boot/verify.h>

int boot_verify_handoff(struct boot_handoff const *handoff)
{
    if (handoff == 0) {
        return 1;
    }

    if (handoff->protocol != BOOT_PROTOCOL_MULTIBOOT2) {
        return 2;
    }

    if ((u32)handoff->loader_magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        return 3;
    }

    if (handoff->loader_info == 0) {
        return 4;
    }

    return 0;
}
