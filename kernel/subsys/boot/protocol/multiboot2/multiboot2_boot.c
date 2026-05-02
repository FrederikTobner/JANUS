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

/// @file multiboot2_boot.c
/// @brief Multiboot2 boot protocol initialization
///
/// Parses the Multiboot2 information structure and populates the boot
/// context. Called from kernel_main via the common boot_init() symbol.
///
/// Multiboot2 passes magic/info via CPU registers (EAX/EBX). These are
/// stashed by assembly (multiboot2_stash_bootinfo) before kernel_main
/// runs. boot_init reads from the stashed values.

#include "multiboot2_protocol.h"

#include <boot/context.h>

/// Kernel image physical end — provided by the multiboot2 linker script
/// (symbol is named kernel_end in the multiboot2 linker, not kernel_phys_end)
extern char kernel_end[];

/// Protocol-private stash — written by assembly via multiboot2_stash_bootinfo,
/// read by boot_init. Correctness relies on write-before-read ordering,
/// not on .bss being zeroed.
static u64 g_mb2_magic;
static void const * g_mb2_info;

/// @brief Stash Multiboot2 boot info from CPU registers
///
/// Called by assembly before kernel_main to save the magic number and
/// info pointer that Multiboot2 provides in EAX/EBX.
///
/// @param magic Value from EAX at boot (should be MULTIBOOT2_BOOTLOADER_MAGIC)
/// @param info  Pointer to the Multiboot2 information structure
void multiboot2_stash_bootinfo(u64 magic, void const * info)
{
    g_mb2_magic = magic;
    g_mb2_info = info;
}

/// @brief Stub — Multiboot2 has no HHDM concept.
///
/// x86_64 serial uses port I/O and ignores all three address parameters,
/// so returning zeros is safe and correct for early serial init.
__cold void boot_early_params(u64 * hhdm_offset, phys_addr_t * kernel_phys_base, virt_addr_t * kernel_virt_base)
{
    *hhdm_offset = 0;
    *kernel_phys_base = 0;
    *kernel_virt_base = 0;
}

/// Parse a Multiboot2 framebuffer tag and populate the display field.
static __cold void parse_framebuffer_tag(boot_context_t * boot_context,
                                         struct multiboot_tag_framebuffer const * fb)
{
    if (fb->fb_type == MULTIBOOT2_FRAMEBUFFER_TYPE_RGB) {
        display_info_t const fb_display = {
            .mode = DISPLAY_MODE_FRAMEBUFFER,
            .framebuffer = (u8 *) (phys_addr_t) fb->addr,
            .width = fb->width,
            .height = fb->height,
            .pitch = fb->pitch,
            .bpp = fb->bpp,
            .red_mask_shift = fb->red_field_position,
            .green_mask_shift = fb->green_field_position,
            .blue_mask_shift = fb->blue_field_position,
        };
        boot_context->display = fb_display;
    } else if (fb->fb_type == MULTIBOOT2_FRAMEBUFFER_TYPE_EGA_TEXT) {
        display_info_t const vga_display = {.mode = DISPLAY_MODE_VGA_TEXT};
        boot_context->display = vga_display;
    }
}

/// Translate a single Multiboot2 memory type to mem_region_type_t.
static mem_region_type_t translate_mmap_type(u32 mb2_type)
{
    switch (mb2_type) {
    case MULTIBOOT2_MEMORY_AVAILABLE:        return MEM_REGION_USABLE;
    case MULTIBOOT2_MEMORY_ACPI_RECLAIMABLE: return MEM_REGION_ACPI_RECLAIMABLE;
    case MULTIBOOT2_MEMORY_NVS:              return MEM_REGION_ACPI_NVS;
    case MULTIBOOT2_MEMORY_BADRAM:           return MEM_REGION_BAD;
    case MULTIBOOT2_MEMORY_RESERVED:
    default:                                 return MEM_REGION_RESERVED;
    }
}

/// Parse a Multiboot2 memory-map tag and populate boot_context->memmap.
static __cold void parse_mmap_tag(boot_context_t * boot_context,
                                   struct multiboot_tag_mmap const * mmap_tag)
{
    u32 const entry_size = mmap_tag->entry_size;
    u8 const * cursor = (u8 const *) mmap_tag->entries;
    u8 const * const end = (u8 const *) mmap_tag + mmap_tag->size;

    while (cursor + entry_size <= end && boot_context->memmap.count < BOOT_MEMMAP_MAX_ENTRIES) {
        struct multiboot_mmap_entry const * src = (struct multiboot_mmap_entry const *) cursor;
        u32 const idx = boot_context->memmap.count;
        boot_context->memmap.entries[idx].base = (phys_addr_t) src->addr;
        boot_context->memmap.entries[idx].length = src->len;
        boot_context->memmap.entries[idx].type = translate_mmap_type(src->type);
        boot_context->memmap.count++;
        cursor += entry_size;
    }
}

/// @brief Initialize the boot context from Multiboot2 info
///
/// Validates the stashed magic number, walks the tag list to extract
/// framebuffer info (if present), and populates the boot context.
/// Sets every field unconditionally to avoid uninitialized reads.
///
/// Multiboot2 is identity-mapped — the HHDM offset is 0 and kernel
/// physical/virtual bases are identical.
///
/// @param boot_context Boot context to populate
/// @return 0 on success, non-zero on failure
__cold error_t boot_init(boot_context_t * boot_context)
{
    if (UNLIKELY((u32) g_mb2_magic != MULTIBOOT2_BOOTLOADER_MAGIC)) {
        return -1;
    }
    if (UNLIKELY(g_mb2_info == NULL)) {
        return -1;
    }

    boot_context->protocol = BOOT_PROTOCOL_MULTIBOOT2;
    boot_context->hhdm_offset = 0;
    boot_context->kernel_phys_base = 0;
    boot_context->kernel_virt_base = 0;
    boot_context->kernel_phys_end = (phys_addr_t) kernel_end;
    boot_context->memmap.count = 0;
    display_info_t const none_display = {.mode = DISPLAY_MODE_NONE};
    boot_context->display = none_display;

    struct multiboot_tag * tag = multiboot_first_tag((struct multiboot_info *) (phys_addr_t) g_mb2_info);
    while (!multiboot_is_end_tag(tag)) {
        if (tag->type == MULTIBOOT2_TAG_FRAMEBUFFER) {
            parse_framebuffer_tag(boot_context, (struct multiboot_tag_framebuffer const *) tag);
        } else if (tag->type == MULTIBOOT2_TAG_MMAP) {
            parse_mmap_tag(boot_context, (struct multiboot_tag_mmap const *) tag);
        }
        tag = multiboot_next_tag(tag);
    }

    return 0;
}
