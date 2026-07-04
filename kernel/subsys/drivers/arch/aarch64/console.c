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

/// @file arch/aarch64/drivers/console.c
/// @brief AArch64 console probe — framebuffer only (no VGA on AArch64).

#include <arch/drivers/console.h>
#include <arch/shared/drivers/fb_console.h>
#include <janus/attributes.h>

__cold console_ops_t const * arch_console_probe(display_info_t const * cfg)
{
    return (cfg->mode == DISPLAY_MODE_FRAMEBUFFER) ? fb_console_init(cfg) : NULL;
}
