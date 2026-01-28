/*****************************************************************************
 * Copyright (C) 2025 by Frederik Tobner                                     *
 *                                                                           *
 * This file is part of JANUS.                                              *
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
 * @file verify.c
 * @brief Multiboot2 boot handoff verification for x86_64
 */

#include <boot/verify.h>
#include <protocol/multiboot2.h>

error_t boot_verify_handoff(u64 loader_magic, void * info)
{
    if ((u32) loader_magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        return -1;
    }

    if (info == 0) {
        return -2;
    }

    return 0;
}
