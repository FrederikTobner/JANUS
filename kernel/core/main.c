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

#include <arch/cpu.h>
#include <boot/handoff.h>
#include <boot/verify.h>
#include <drivers/uart.h>
#include <drivers/vga_text.h>
#include <tinyos/types.h>
#include <tinyos/version.h>

#define TINYOS_HELLO_MESSAGE                                     \
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
 * - CPU is in 32-bit protected mode
 * - Interrupts are disabled
 * - Paging is disabled
 *
 * @param handoff Minimal boot handoff structure
 */
void kernel_main(struct boot_handoff const * handoff)
{
    if (boot_verify_handoff(handoff) != 0) {
        cpu_halt_forever();
    }

    // Initialize VGA text output and print a simple message
    vga_text_init();
    // Green font color on black background
    vga_text_set_color(2, 0);
    vga_text_write_string(TINYOS_HELLO_MESSAGE);
    vga_text_write_string("\nTinyOS Version:"
                          " " TINYOS_VERSION_STRING "\n\n");

    // Initialize serial port for debugging output
    if (uart_init() == 0) {
        // If the serial port initialized successfully, print something lul
        uart_write_string(TINYOS_HELLO_MESSAGE);
    }

    for (;;) {
        cpu_halt();
    }
}
