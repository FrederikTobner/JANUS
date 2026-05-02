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

/// @file limine_boot.c
/// @brief Limine boot protocol initialization
///
/// Reads Limine request responses and populates the boot context.
/// Called from kernel_main via the common boot_init() symbol.

#include "limine_protocol.h"

#include <boot/context.h>

// Extern request symbols (defined in limine_requests.c)

extern volatile struct limine_hhdm_request limine_hhdm_request;
extern volatile struct limine_framebuffer_request limine_framebuffer_request;
extern volatile struct limine_executable_address_request limine_executable_address_request;
extern volatile struct limine_memmap_request limine_memmap_request;

/// Kernel image physical end — provided by the linker script
extern char kernel_phys_end[];

/// @brief Query address-translation parameters directly from Limine request structs.
///
/// The Limine bootloader populates the response pointers before kernel_main
/// is entered, so this is safe to call before boot_init().
__cold void boot_early_params(u64 * hhdm_offset, phys_addr_t * kernel_phys_base, virt_addr_t * kernel_virt_base)
{
    struct limine_hhdm_response const * hhdm = limine_hhdm_request.response;
    *hhdm_offset = (hhdm != NULL) ? hhdm->offset : 0;
    *kernel_phys_base = 0;
    *kernel_virt_base = 0;
    struct limine_executable_address_response const * exe = limine_executable_address_request.response;
    if (exe != NULL) {
        *kernel_phys_base = exe->physical_base;
        *kernel_virt_base = exe->virtual_base;
    }
}

/// @brief Initialize the boot context from Limine responses
///
/// Reads the response pointers from each Limine request structure,
/// validates them, and populates the boot context. Sets every field
/// unconditionally to avoid uninitialized reads.
///
/// @param boot_context Boot context to populate
/// @return 0 on success, non-zero on failure (missing required responses)
__cold error_t boot_init(boot_context_t * boot_context)
{
    // Set safe defaults for ALL fields unconditionally
    boot_context->protocol = BOOT_PROTOCOL_LIMINE;
    boot_context->hhdm_offset = 0;
    boot_context->kernel_phys_base = 0;
    boot_context->kernel_virt_base = 0;
    boot_context->kernel_phys_end = 0;
    boot_context->memmap.count = 0;
    display_info_t const none_display = {.mode = DISPLAY_MODE_NONE};
    boot_context->display = none_display;

    // HHDM offset is required for address translation
    struct limine_hhdm_response const * hhdm = limine_hhdm_request.response;
    if (UNLIKELY(hhdm == NULL)) {
        return -1;
    }
    boot_context->hhdm_offset = hhdm->offset;

    // Executable address (required on aarch64, optional on x86_64)
    struct limine_executable_address_response const * executable_address = limine_executable_address_request.response;
    if (executable_address != NULL) {
        boot_context->kernel_phys_base = executable_address->physical_base;
        boot_context->kernel_virt_base = executable_address->virtual_base;
    }

    // Framebuffer is optional — if not present, display_mode remains NONE
    struct limine_framebuffer_response const * framebuffer_response = limine_framebuffer_request.response;
    if (framebuffer_response != NULL && framebuffer_response->framebuffer_count > 0) {
        struct limine_framebuffer const * primary_framebuffer = framebuffer_response->framebuffers[0];
        display_info_t const fb_display = {
            .mode = DISPLAY_MODE_FRAMEBUFFER,
            .framebuffer = (u8 *) primary_framebuffer->address,
            .width = primary_framebuffer->width,
            .height = primary_framebuffer->height,
            .pitch = primary_framebuffer->pitch,
            .bpp = primary_framebuffer->bpp,
            .red_mask_shift = primary_framebuffer->red_mask_shift,
            .green_mask_shift = primary_framebuffer->green_mask_shift,
            .blue_mask_shift = primary_framebuffer->blue_mask_shift,
        };
        boot_context->display = fb_display;
    }

    // Physical end of the kernel image (linker-provided symbol)
    boot_context->kernel_phys_end = (phys_addr_t) kernel_phys_end;

    // Physical memory map
    struct limine_memmap_response const * memmap_response = limine_memmap_request.response;
    if (memmap_response != NULL) {
        u64 const max_entries = BOOT_MEMMAP_MAX_ENTRIES;
        u64 const n = (memmap_response->entry_count < max_entries) ? memmap_response->entry_count : max_entries;
        for (u64 i = 0; i < n; i++) {
            struct limine_memmap_entry const * src = memmap_response->entries[i];
            boot_context->memmap.entries[i].base = (phys_addr_t) src->base;
            boot_context->memmap.entries[i].length = src->length;
            boot_context->memmap.entries[i].type = (mem_region_type_t) src->type;
        }
        boot_context->memmap.count = (u32) n;
    }

    return 0;
}
