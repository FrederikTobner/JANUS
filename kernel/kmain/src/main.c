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

/// @file main.c
/// @brief JANUS Kernel Entry Point
///
/// Called from the assembly entry point after minimal hardware setup.
/// Allocates the kernel descriptor on the stack, calls boot_init to
/// populate it, then initializes subsystems and enters the main loop.

#include <asm/cpu.h>
#include <boot/context.h>
#include <janus/attributes.h>
#include <janus/config.h>
#include <janus/errno.h>
#include <janus/types.h>
#include <kio/kio.h>
#include <kmain/console.h>
#include <kmain/kernel_descriptor.h>
#include <mm/pmm.h>

// Greeting message printed after booting has been completed and the console has ben initialized
#define JANUS_HELLO_MESSAGE                  \
    "     _   _    _   _ _   _ ____  \n"     \
    "    | | / \\  | \\ | | | | / ___|\n"    \
    " _  | |/ _ \\ |  \\| | | | \\___ \\ \n" \
    "| |_| / ___ \\| |\\  | |_| |___) |\n"   \
    " \\___/_/   \\_\\_| \\_|\\___/|____/ \n"

/// @brief Main kernel entry point
///
/// Called from the assembly entry point. At this point:
/// - Stack is configured
/// - CPU is in 64-bit long mode (x86_64) or EL1 (aarch64)
/// - Interrupts are disabled
/// - For Multiboot2: boot info has been stashed via multiboot2_stash_bootinfo
///
/// Allocates kernel_descriptor_t on the stack, populates it via boot_init,
/// then passes the boot boot_context slice to each subsystem initializer.
__noreturn void kernel_main(void)
{
    kernel_descriptor_t descriptor;
    if (boot_init(&descriptor.boot) != JANUS_OK) {
        kpanic("boot_init failed — cannot continue");
    }

    console_init(&descriptor.boot);
    // Print greeting
    kprintf("%s\nVersion: %s\n\n", JANUS_HELLO_MESSAGE, JANUS_VERSION_STRING);

    // Initialize physical memory manager
    error_t pmm_err =
        mm_pmm_init(&descriptor.boot.memmap, descriptor.boot.kernel_phys_base, descriptor.boot.kernel_phys_end);
    if (pmm_err != JANUS_OK) {
        kpanic("mm_pmm_init failed: %d", pmm_err);
    }
    mm_pmm_stats_t pmm_stats;
    mm_pmm_get_stats(&pmm_stats);
    kprintf("PMM: %llu MiB free\n", pmm_stats.free_pages * 4096ULL / (1024ULL * 1024ULL));

    // Halt the CPU forever
    asm_cpu_halt_forever();
}
