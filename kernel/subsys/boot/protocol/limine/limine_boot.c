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
#include <janus/errno.h>

// Extern request symbols (defined in limine_requests.c)
extern limine_hhdm_request_t volatile limine_hhdm_request;
extern limine_framebuffer_request_t volatile limine_framebuffer_request;
extern limine_executable_address_request_t volatile limine_executable_address_request;
extern limine_memmap_request_t volatile limine_memmap_request;

/// Translate Limine memmap entry type to canonical mem_region_type_t.
static mem_region_type_t limine_translate_memmap_type(u64 limine_type);

extern char kernel_phys_start[];

extern char kernel_size[];

/// @brief Query address-translation parameters directly from Limine request structs.
///
/// The Limine bootloader populates the response pointers before kernel_main
/// is entered, so this is safe to call before boot_init().
__cold void boot_early_params(u64 * hhdm_offset, phys_addr_t * kernel_phys_base, virt_addr_t * kernel_virt_base)
{
    limine_hhdm_response_t const * hhdm = limine_hhdm_request.response;
    *hhdm_offset = (hhdm != NULL) ? hhdm->offset : 0;
    *kernel_phys_base = 0;
    *kernel_virt_base = 0;
    limine_executable_address_response_t const * exe = limine_executable_address_request.response;
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
    boot_context->kernel_phys_end = (phys_addr_t) kernel_phys_start;

    // HHDM offset is required for address translation
    limine_hhdm_response_t const * hhdm = limine_hhdm_request.response;
    if (UNLIKELY(hhdm == NULL)) {
        return JANUS_ENODEV; // HHDM is mandatory - bootloader did not provide it
    }
    boot_context->hhdm_offset = hhdm->offset;

    // Executable address (required on aarch64, optional on x86_64)
    limine_executable_address_response_t const * executable_address = limine_executable_address_request.response;
    if (executable_address != NULL) {
        boot_context->kernel_phys_base = executable_address->physical_base;
        boot_context->kernel_virt_base = executable_address->virtual_base;
    }

    // Framebuffer is optional — if not present, display_mode remains NONE
    limine_framebuffer_response_t const * framebuffer_response = limine_framebuffer_request.response;
    if (framebuffer_response != NULL && framebuffer_response->framebuffer_count > 0) {
        limine_framebuffer_t const * primary_framebuffer = framebuffer_response->framebuffers[0];
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
    boot_context->kernel_phys_end = (phys_addr_t) (boot_context->kernel_phys_base + (uintptr_t) kernel_size);

    // Physical memory map
    limine_memmap_response_t const * memmap_response = limine_memmap_request.response;
    if (memmap_response != NULL) {
        u64 const max_entries = BOOT_MEMMAP_MAX_ENTRIES;
        u64 const n = (memmap_response->entry_count < max_entries) ? memmap_response->entry_count : max_entries;
        for (u64 i = 0; i < n; i++) {
            limine_memmap_entry_t const * src = memmap_response->entries[i];
            boot_context->memmap.entries[i].base = (phys_addr_t) src->base;
            boot_context->memmap.entries[i].length = src->length;
            boot_context->memmap.entries[i].type = limine_translate_memmap_type(src->type);
        }
        boot_context->memmap.count = (u32) n;
    }

    return JANUS_OK;
}

static mem_region_type_t limine_translate_memmap_type(u64 limine_type)
{
    switch (limine_type) {
    case LIMINE_MEMMAP_USABLE:
        return MEM_REGION_USABLE;
    case LIMINE_MEMMAP_RESERVED:
        return MEM_REGION_RESERVED;
    case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
        return MEM_REGION_ACPI_RECLAIMABLE;
    case LIMINE_MEMMAP_ACPI_NVS:
        return MEM_REGION_ACPI_NVS;
    case LIMINE_MEMMAP_BAD_MEMORY:
        return MEM_REGION_BAD;
    case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
        return MEM_REGION_BOOTLOADER_RECLAIMABLE;
    case LIMINE_MEMMAP_KERNEL_AND_MODULES:
        return MEM_REGION_KERNEL_AND_MODULES;
    case LIMINE_MEMMAP_FRAMEBUFFER:
        return MEM_REGION_FRAMEBUFFER;
    default:
        return MEM_REGION_RESERVED;
    }
}
