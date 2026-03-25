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
 * @file multiboot2_boot.c
 * @brief Multiboot2 boot protocol initialization
 *
 * Parses the Multiboot2 information structure and populates the boot
 * context. Called from kernel_main via the common boot_init() symbol.
 *
 * Multiboot2 passes magic/info via CPU registers (EAX/EBX). These are
 * stashed by assembly (multiboot2_stash_bootinfo) before kernel_main
 * runs. boot_init reads from the stashed values.
 */

#include "multiboot2_protocol.h"

#include <boot/context.h>

/// Protocol-private stash — written by assembly via multiboot2_stash_bootinfo,
/// read by boot_init. Correctness relies on write-before-read ordering,
/// not on .bss being zeroed.
static u64 g_mb2_magic;
static void const * g_mb2_info;

/**
 * @brief Stash Multiboot2 boot info from CPU registers
 *
 * Called by assembly before kernel_main to save the magic number and
 * info pointer that Multiboot2 provides in EAX/EBX.
 *
 * @param magic Value from EAX at boot (should be MULTIBOOT2_BOOTLOADER_MAGIC)
 * @param info  Pointer to the Multiboot2 information structure
 */
void multiboot2_stash_bootinfo(u64 magic, void const * info)
{
    g_mb2_magic = magic;
    g_mb2_info = info;
}

/**
 * @brief Initialize the boot context from Multiboot2 info
 *
 * Validates the stashed magic number, walks the tag list to extract
 * framebuffer info (if present), and populates the boot context.
 * Sets every field unconditionally to avoid uninitialized reads.
 *
 * Multiboot2 is identity-mapped — the HHDM offset is 0 and kernel
 * physical/virtual bases are identical.
 *
 * @param boot_context Boot context to populate
 * @return 0 on success, non-zero on failure
 */
int boot_init(boot_context_t * boot_context)
{
    if ((u32) g_mb2_magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        return -1;
    }
    if (g_mb2_info == NULL) {
        return -1;
    }

    // Set safe defaults for ALL fields unconditionally
    boot_context->protocol = BOOT_PROTOCOL_MULTIBOOT2;
    boot_context->hhdm_offset = 0;
    boot_context->kernel_phys_base = 0;
    boot_context->kernel_virt_base = 0;
    boot_context->has_display = false;

    // Walk tag list looking for framebuffer
    struct multiboot_tag * tag = multiboot_first_tag((struct multiboot_info *) (phys_addr_t) g_mb2_info);
    while (!multiboot_is_end_tag(tag)) {
        if (tag->type == MULTIBOOT2_TAG_FRAMEBUFFER) {
            struct multiboot_tag_framebuffer const * fb = (struct multiboot_tag_framebuffer const *) tag;

            // Only use a direct-color (RGB) framebuffer.
            // EGA text mode (fb_type 2) and indexed colour (fb_type 0)
            // are not usable as a linear framebuffer.
            if (fb->fb_type == MULTIBOOT2_FRAMEBUFFER_TYPE_RGB) {
                boot_context->display = (boot_display_info_t) {
                    .framebuffer = (u8 *) (phys_addr_t) fb->addr,
                    .width = fb->width,
                    .height = fb->height,
                    .pitch = fb->pitch,
                    .bpp = fb->bpp,
                    .red_mask_shift = fb->red_field_position,
                    .green_mask_shift = fb->green_field_position,
                    .blue_mask_shift = fb->blue_field_position,
                };
                boot_context->has_display = true;
            }
            break;
        }
        tag = multiboot_next_tag(tag);
    }

    return 0;
}
