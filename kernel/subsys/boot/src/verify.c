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
 * @brief Boot handoff verification for multiple protocols
 *
 * Supports:
 * - Multiboot2 (GRUB)
 * - Limine
 */

#include <boot/verify.h>
#include <internal/protocol/limine.h>
#include <internal/protocol/multiboot2.h>

error_t boot_verify_handoff(u64 loader_magic, void * info)
{
    // Check for Multiboot2 magic (32-bit, upper 32 bits should be zero)
    if ((u32) loader_magic == MULTIBOOT2_BOOTLOADER_MAGIC) {
        if (info == NULL) {
            return -2; // Multiboot2 info pointer is NULL
        }
        return 0; // Valid Multiboot2 handoff
    }

    // Check for Limine magic
    if (loader_magic == LIMINE_BOOTLOADER_MAGIC) {
        if (info == NULL) {
            return -2; // Limine info pointer is NULL
        }
        return 0; // Valid Limine handoff
    }

    // Unknown bootloader magic
    return -1;
}
