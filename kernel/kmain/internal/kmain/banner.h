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

#ifndef KMAIN_BANNER_H
#define KMAIN_BANNER_H

/**
 * @file banner.h
 * @brief Kernel startup banner display.
 *
 * Displays the JANUS ASCII art logo and version information.
 */

#include <janus/types.h>

/**
 * @brief Print the startup greeting message.
 *
 * Displays the JANUS ASCII art banner and version string on all available
 * output devices (serial and/or TTY).
 *
 * @param serial_available Whether serial output is available
 * @param tty_available Whether TTY output is available
 */
void kbanner_print(bool serial_available, bool tty_available);

#endif /* KMAIN_BANNER_H */
