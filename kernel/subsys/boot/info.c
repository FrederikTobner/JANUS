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
 * @file info.c
 * @brief Boot information storage and retrieval
 *
 * Stores boot information from different boot protocols in a unified format.
 */

#include <boot/info.h>
#include <protocol/limine.h>
#include <protocol/multiboot2.h>

/* Boot information storage */
static boot_protocol_t g_boot_protocol = BOOT_PROTOCOL_UNKNOWN;
static u64 g_hhdm_offset = 0;

error_t boot_info_init(u64 loader_magic, void * info)
{
    /* Check for Multiboot2 magic (32-bit) */
    if ((u32) loader_magic == MULTIBOOT2_BOOTLOADER_MAGIC) {
        g_boot_protocol = BOOT_PROTOCOL_MULTIBOOT2;
        /* Multiboot2 with our identity-mapped boot has HHDM offset of 0 */
        g_hhdm_offset = 0;
        (void) info; /* Multiboot2 info not used for HHDM */
        return 0;
    }

    /* Check for Limine magic */
    if (loader_magic == LIMINE_BOOTLOADER_MAGIC) {
        g_boot_protocol = BOOT_PROTOCOL_LIMINE;

        /*
         * For Limine, 'info' points to a limine_hhdm_response structure
         * that was set by the Limine entry point.
         */
        if (info != NULL) {
            struct limine_hhdm_response * hhdm_response = 
                (struct limine_hhdm_response *) info;
            g_hhdm_offset = hhdm_response->offset;
        } else {
            /* No HHDM info provided - this is a problem */
            return -3;
        }
        return 0;
    }

    /* Unknown bootloader */
    return -1;
}

boot_protocol_t boot_info_get_protocol(void)
{
    return g_boot_protocol;
}

u64 boot_info_get_hhdm_offset(void)
{
    return g_hhdm_offset;
}
