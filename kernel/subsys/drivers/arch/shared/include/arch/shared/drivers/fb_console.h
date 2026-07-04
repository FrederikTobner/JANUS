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

#ifndef ARCH_SHARED_DRIVERS_FB_CONSOLE_H
#define ARCH_SHARED_DRIVERS_FB_CONSOLE_H

/// @file arch/shared/drivers/fb_console.h
/// @brief Framebuffer text-console backend.
///
/// Declares fb_console_init(), which initializes the shared framebuffer
/// console and returns its console_ops_t.  Used by arch_console_probe()
/// on any architecture that supports a linear framebuffer.

#include <arch/drivers/console.h>
#include <contracts/display.h>

/// @brief Initialize the framebuffer console for the given display config.
///
/// @param cfg  Display configuration (must have mode == DISPLAY_MODE_FRAMEBUFFER).
/// @return Pointer to the framebuffer console's console_ops_t.
console_ops_t const * fb_console_init(display_info_t const * cfg);

#endif /* ARCH_SHARED_DRIVERS_FB_CONSOLE_H */
