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

/// @file arch/x86_64/drivers/console.c
/// @brief x86_64 console probe — selects VGA text or framebuffer backend.

#include <arch/drivers/console.h>
#include <arch/internal/drivers/vga_console.h>
#include <arch/shared/drivers/fb_console.h>
#include <janus/attributes.h>

__cold console_ops_t const * arch_console_probe(display_info_t const * cfg)
{
    switch (cfg->mode) {
    case DISPLAY_MODE_VGA_TEXT:
        return vga_console_init(cfg);
    case DISPLAY_MODE_FRAMEBUFFER:
        return fb_console_init(cfg);
    default:
        return NULL;
    }
}
