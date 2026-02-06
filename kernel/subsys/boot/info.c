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

#include <boot/arch_requests.h>
#include <boot/info.h>
#include <internal/protocol/limine.h>
#include <internal/protocol/multiboot2.h>

/* Boot information storage */
static boot_protocol_t g_boot_protocol = BOOT_PROTOCOL_UNKNOWN;
static u64 g_hhdm_offset = 0;
static u64 g_kernel_phys_base = 0;
static u64 g_kernel_virt_base = 0;

error_t boot_info_init(u64 loader_magic, void * info)
{
    // Set up protocol-specific boot requests via boot arch layer
    struct boot_info_requests boot_requests;
    boot_requests.limine_executable_address_request = NULL;
    boot_arch_setup_requests(&boot_requests);

    /* Check for Multiboot2 magic (32-bit) */
    if ((u32) loader_magic == MULTIBOOT2_BOOTLOADER_MAGIC) {
        g_boot_protocol = BOOT_PROTOCOL_MULTIBOOT2;
        g_hhdm_offset = 0;
        (void) info;
        return 0;
    }

    /* Check for Limine magic */
    if (loader_magic == LIMINE_BOOTLOADER_MAGIC) {
        g_boot_protocol = BOOT_PROTOCOL_LIMINE;
        if (info != NULL) {
            struct limine_hhdm_response * hhdm_response = (struct limine_hhdm_response *) info;
            g_hhdm_offset = hhdm_response->offset;
        } else {
            return -3;
        }

        boot_arch_extract_executable_address(&g_kernel_phys_base, &g_kernel_virt_base);
        return 0;
    }
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

u64 boot_info_get_kernel_phys_base(void)
{
    return g_kernel_phys_base;
}

u64 boot_info_get_kernel_virt_base(void)
{
    return g_kernel_virt_base;
}
