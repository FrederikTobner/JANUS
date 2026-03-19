/*****************************************************************************
 * Copyright (C) 2025 by Frederik Tobner                                     *
 *                                                                           *
 * This file is part of JANUS.                                               *
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
 * Called from the assembly entry point after minimal hardware setup.
 * Allocates the kernel descriptor on the stack, calls boot_init to
 * populate it, then initializes subsystems and enters the main loop.
 */

#include <boot/context.h>
#include <drivers/cpu.h>
#include <janus/attributes.h>
#include <janus/types.h>
#include <kmain/banner.h>
#include <kmain/init.h>
#include <kmain/kernel_descriptor.h>

/**
 * @brief Main kernel entry point
 *
 * Called from the assembly entry point. At this point:
 * - Stack is configured
 * - CPU is in 64-bit long mode (x86_64) or EL1 (aarch64)
 * - Interrupts are disabled
 * - For Multiboot2: boot info has been stashed via multiboot2_stash_bootinfo
 *
 * Allocates kernel_descriptor_t on the stack, populates it via boot_init,
 * then passes the boot context slice to each subsystem initializer.
 */
__noreturn void kernel_main(void)
{
    kernel_descriptor_t kd;
    if (boot_init(&kd.boot) != 0) {
        drivers_cpu_halt_forever();
    }

    boot_context_t const * ctx = &kd.boot;

    // Initialize drivers
    bool serial_available = kinit_serial(ctx);
    bool tty_available = kinit_tty(ctx, serial_available);

    // Print greeting
    kbanner_print(serial_available, tty_available);

    // Halt the CPU forever
    drivers_cpu_halt_forever();
}
