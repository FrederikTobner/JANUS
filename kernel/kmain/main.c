/*****************************************************************************
 * Copyright (C) 2025 by Frederik Tobner                                     *
 *                                                                           *
 * This file is part of JANUS.                                             *
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

/**
 * @file main.c
 * @brief JANUS Kernel Entry Point
 *
 * This is called by boot.asm after the bootloader transfers control.
 */

#include <boot/info.h>
#include <boot/verify.h>
#include <drivers/cpu.h>
#include <drivers/serial.h>
#include <drivers/tty.h>
#include <janus/attributes.h>
#include <janus/types.h>
#include <janus/version.h>

#define JANUS_HELLO_MESSAGE                                      \
    " _   _      _ _         _    _            _     _ _ \n"     \
    "| | | |    | | |       | |  | |          | |   | | |\n"     \
    "| |_| | ___| | | ___   | |  | | ___  _ __| | __| | |\n"     \
    "|  _  |/ _ \\ | |/ _ \\  | |/\\| |/ _ \\| '__| |/ _` | |\n" \
    "| | | |  __/ | | (_) | \\  /\\  / (_) | |  | | (_| |_|\n"   \
    "\\_| |_/\\___|_|_|\\___/   \\/  \\/ \\___/|_|  |_|\\__,_(_)\n"

/**
 * @brief Limine framebuffer structure
 *
 * Matches the Limine protocol framebuffer structure.
 * See: https://github.com/limine-bootloader/limine/blob/trunk/PROTOCOL.md
 */
struct limine_framebuffer {
    void * address;
    u64 width;
    u64 height;
    u64 pitch;
    u16 bpp;
    u8 memory_model;
    u8 red_mask_size;
    u8 red_mask_shift;
    u8 green_mask_size;
    u8 green_mask_shift;
    u8 blue_mask_size;
    u8 blue_mask_shift;
    u8 unused[7];
    u64 edid_size;
    void * edid;
    /* Mode count and modes follow in revision 1+ */
};

/**
 * @brief Limine framebuffer response structure
 */
struct limine_framebuffer_response {
    u64 revision;
    u64 framebuffer_count;
    struct limine_framebuffer ** framebuffers;
};

/**
 * @brief Main kernel entry point
 *
 * Called from boot.asm after initial setup. At this point:
 * - Stack is configured (16 KiB)
 * - CPU is in 64-bit long mode
 * - Interrupts are disabled
 * - Paging is disabled
 *
 * @param loader_magic Magic value identifying the boot protocol
 * @param info Boot protocol specific info (Multiboot2 info or HHDM response)
 * @param fb_info Framebuffer response (Limine only, NULL for Multiboot2)
 */
__noreturn void kernel_main(u64 loader_magic, void * info, void * fb_info)
{
    if (boot_verify_handoff(loader_magic, info) != 0) {
        // Disable interrupts and halt forever on boot verification failure
        cpu_halt_forever();
    }

    // Initialize boot info (must be done before drivers that need HHDM offset)
    if (boot_info_init(loader_magic, info) != 0) {
        cpu_halt_forever();
    }

    // Get HHDM offset for drivers that access physical memory
    u64 hhdm_offset = boot_info_get_hhdm_offset();

    bool serial_initialized = false;
    // Initialize serial port for debugging output
    if (serial_init() == 0) {
        serial_puts("Serial driver initialized\n");
        serial_initialized = true;
    }

    // Initialize TTY output
    bool tty_available = false;
    tty_display_config_t tty_config;
    tty_config.framebuffer = NULL;

    if (hhdm_offset == 0) {
        // Identity-mapped (Multiboot2): VGA buffer is directly accessible
        // Pass NULL config to use VGA text mode
        if (tty_init(NULL) == 0) {
            tty_available = true;
            if (serial_initialized) {
                serial_puts("TTY driver initialized (VGA text mode)\n");
            }
        }
    } else if (fb_info != NULL) {
        // Higher-half mapped (Limine): Use framebuffer for text output
        struct limine_framebuffer_response * fb_resp =
            (struct limine_framebuffer_response *)fb_info;

        if (fb_resp->framebuffer_count > 0 && fb_resp->framebuffers != NULL) {
            struct limine_framebuffer * fb = fb_resp->framebuffers[0];

            // Set up display config from Limine framebuffer
            tty_config.framebuffer = (u8 *)fb->address;
            tty_config.width = fb->width;
            tty_config.height = fb->height;
            tty_config.pitch = fb->pitch;
            tty_config.bpp = fb->bpp;
            tty_config.red_mask_shift = fb->red_mask_shift;
            tty_config.green_mask_shift = fb->green_mask_shift;
            tty_config.blue_mask_shift = fb->blue_mask_shift;

            if (tty_init(&tty_config) == 0) {
                tty_available = true;
                if (serial_initialized) {
                    serial_puts("TTY driver initialized (framebuffer mode)\n");
                }
            }
        } else {
            if (serial_initialized) {
                serial_puts("TTY skipped (no framebuffer available)\n");
            }
        }
    } else {
        if (serial_initialized) {
            serial_puts("TTY skipped (Limine without framebuffer)\n");
        }
    }

    // Print greeting
    if (tty_available) {
        // Green font color on black background
        tty_set_color(2, 0);
        tty_puts(JANUS_HELLO_MESSAGE);
        tty_puts("\nJANUS Version:"
                 " " JANUS_VERSION_STRING "\n\n");
    }
    if (serial_initialized) {
        serial_puts(JANUS_HELLO_MESSAGE);
        serial_puts("\nJANUS Version: " JANUS_VERSION_STRING "\n\n");
    }

    // Halt the CPU forever
    for (;;) {
        cpu_halt();
    }
}
