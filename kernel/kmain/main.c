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
 * The actual initialization logic is delegated to separate modules
 * for clarity.
 */

#include <boot/info.h>
#include <boot/verify.h>
#include <drivers/cpu.h>
#include <janus/attributes.h>
#include <janus/types.h>
#include <kmain/banner.h>
#include <kmain/init.h>

/**
 * @brief Main kernel entry point
 *
 * Called from boot.asm after initial setup. At this point:
 * - Stack is configured (16 KiB)
 * - CPU is in 64-bit long mode
 * - Interrupts are disabled
 * - Paging is disabled (Multiboot2) or enabled with HHDM (Limine)
 *
 * @param loader_magic Magic value identifying the boot protocol
 * @param info Boot protocol specific info (Multiboot2 info or HHDM response)
 * @param fb_info Framebuffer response (Limine only, NULL for Multiboot2)
 */
__noreturn void kernel_main(u64 loader_magic, void * info, void * fb_info)
{
    // Verify boot handoff
    if (boot_verify_handoff(loader_magic, info) != 0) {
        cpu_halt_forever();
    }

    // Initialize boot info (must be done before drivers that need HHDM offset)
    if (boot_info_init(loader_magic, info) != 0) {
        cpu_halt_forever();
    }

    // Initialize drivers
    bool serial_available = kinit_serial();
    bool tty_available = kinit_tty(boot_info_get_hhdm_offset(), fb_info, serial_available);

    // Print greeting
    kbanner_print(serial_available, tty_available);

    // Halt the CPU forever
    cpu_halt_forever();
}
