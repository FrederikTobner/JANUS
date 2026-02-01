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
 * @file banner.c
 * @brief Kernel startup banner display.
 *
 * Contains the ASCII art logo and version display routines.
 */

#include <kmain/banner.h>

#include <drivers/serial.h>
#include <drivers/tty.h>
#include <janus/version.h>

/* -------------------------- Private Constants -------------------------- */

#define JANUS_HELLO_MESSAGE                                      \
    " _   _      _ _         _    _            _     _ _ \n"     \
    "| | | |    | | |       | |  | |          | |   | | |\n"     \
    "| |_| | ___| | | ___   | |  | | ___  _ __| | __| | |\n"     \
    "|  _  |/ _ \\ | |/ _ \\  | |/\\| |/ _ \\| '__| |/ _` | |\n" \
    "| | | |  __/ | | (_) | \\  /\\  / (_) | |  | | (_| |_|\n"   \
    "\\_| |_/\\___|_|_|\\___/   \\/  \\/ \\___/|_|  |_|\\__,_(_)\n"

/* -------------------------- Public Functions -------------------------- */

void kbanner_print(bool serial_available, bool tty_available)
{
    if (tty_available) {
        // Green font color on black background
        drivers_tty_set_color(2, 0);
        drivers_tty_puts(JANUS_HELLO_MESSAGE);
        drivers_tty_puts("\nJANUS Version: " JANUS_VERSION_STRING "\n\n");
    }

    if (serial_available) {
        drivers_serial_puts(JANUS_HELLO_MESSAGE);
        drivers_serial_puts("\nJANUS Version: " JANUS_VERSION_STRING "\n\n");
    }
}
