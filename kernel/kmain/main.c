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
 * @brief Main kernel entry point
 *
 * Called from boot.asm after initial setup. At this point:
 * - Stack is configured (16 KiB)
 * - CPU is in 64-bit long mode
 * - Interrupts are disabled
 * - Paging is disabled
 *
 * @param handoff Minimal boot handoff structure
 */
__noreturn void kernel_main(u64 loader_magic, void * info)
{
    if (boot_verify_handoff(loader_magic, info) != 0) {
        // Disable interrupts and halt forever on boot verification failure
        cpu_halt_forever();
    }
    bool serial_initialized = false;
    // Initialize serial port for debugging output
    if (serial_init() == 0) {
        // If the serial port initialized successfully, print something lul
        serial_puts("Serial driver initialized\n");
        serial_initialized = true;
    }
    // Initialize TTY output and print a simple message
    tty_init();
    if (serial_initialized) {
        serial_puts("TTY driver initialized\n");
    }
    // Green font color on black background
    tty_set_color(2, 0);
    tty_puts(JANUS_HELLO_MESSAGE);
    tty_puts("\nJANUS Version:"
             " " JANUS_VERSION_STRING "\n\n");

    // Halt the CPU forever
    for (;;) {
        cpu_halt();
    }
}
