/*****************************************************************************
 * Copyright (C) 2025 by Frederik Tobner                                     *
 *                                                                           *
 * This file is part of TinyOS.                                             *
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
 * @brief TinyOS Kernel Entry Point
 *
 * This is called by boot.asm after the bootloader transfers control.
 */

#include <boot/multiboot.h>
#include <tinyos/types.h>

/**
 * @brief Main kernel entry point
 *
 * Called from boot.asm after initial setup. At this point:
 * - Stack is configured (16 KiB)
 * - CPU is in 32-bit protected mode
 * - Interrupts are disabled
 * - Paging is disabled
 *
 * @param magic Multiboot2 magic number (should be 0x36d76289)
 * @param info Pointer to multiboot information structure
 */
void kernel_main(uint32_t magic, struct multiboot_info * info)
{
    // Verify we were loaded by a Multiboot2-compliant bootloader
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        // Can't print error yet - just halt
        // Invalid bootloader, hang the system
        for (;;) {
            __asm__ volatile("cli; hlt");
        }
    }

    // Verify multiboot info pointer is valid
    if (info == 0) {
        // Invalid multiboot info, hang
        for (;;) {
            __asm__ volatile("cli; hlt");
        }
    }

    // TODO (T7): Initialize serial output
    // TODO (T7): Print boot success message
    // TODO: Parse multiboot information structure
    // TODO: Initialize memory management
    // TODO: Set up interrupt handling

    // Kernel initialization complete - infinite loop for now
    // In the future, this will hand off to the scheduler
    for (;;) {
        __asm__ volatile("hlt");
    }
}
