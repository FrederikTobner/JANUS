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
#include <drivers/serial.h>
#include <drivers/vga_text.h>
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
void kernel_main(u32 magic, struct multiboot_info *info) {
  // Verify we were loaded by a Multiboot2-compliant bootloader
  if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
    for (;;) {
      __asm__ volatile("cli; hlt");
    }
  }

  // Verify multiboot information structure is present
  if (info == 0) {
    for (;;) {
      __asm__ volatile("cli; hlt");
    }
  }

  // Initialize VGA text output and print a simple message
  vga_text_init();
// Green on black background
  vga_text_set_color(2, 0);
  vga_text_write_string(" _   _      _ _         _    _            _     _ _ \n"
        "| | | |    | | |       | |  | |          | |   | | |\n"
        "| |_| | ___| | | ___   | |  | | ___  _ __| | __| | |\n"
        "|  _  |/ _ \\ | |/ _ \\  | |/\\| |/ _ \\| '__| |/ _` | |\n"
        "| | | |  __/ | | (_) | \\  /\\  / (_) | |  | | (_| |_|\n"
        "\\_| |_/\\___|_|_|\\___/   \\/  \\/ \\___/|_|  |_|\\__,_(_)\n");

  // Initialize serial port for debugging output
  if (serial_init() == 0) {
    // If the serial port initialized successfully, print something lul
    serial_write_string(
        " _   _      _ _         _    _            _     _ _ \n"
        "| | | |    | | |       | |  | |          | |   | | |\n"
        "| |_| | ___| | | ___   | |  | | ___  _ __| | __| | |\n"
        "|  _  |/ _ \\ | |/ _ \\  | |/\\| |/ _ \\| '__| |/ _` | |\n"
        "| | | |  __/ | | (_) | \\  /\\  / (_) | |  | | (_| |_|\n"
        "\\_| |_/\\___|_|_|\\___/   \\/  \\/ \\___/|_|  |_|\\__,_(_)\n");
  }

  for (;;) {
    __asm__ volatile("hlt");
  }
}
