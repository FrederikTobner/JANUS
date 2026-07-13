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

#ifndef JANUS_MULTIBOOT2_PROTOCOL_H
#define JANUS_MULTIBOOT2_PROTOCOL_H

/// @file multiboot2_protocol.h
/// @brief Multiboot2 boot protocol type definitions
///
/// Constants and structures for the Multiboot2 protocol.
/// See: https://www.gnu.org/software/grub/manual/multiboot2/multiboot.html

#include <janus/attributes.h>
#include <janus/types.h>

/// Multiboot2 magic value passed by bootloader in EAX
#define MULTIBOOT2_BOOTLOADER_MAGIC             0x36d76289

/// Architecture types
#define MULTIBOOT2_ARCHITECTURE_I386            0
#define MULTIBOOT2_ARCHITECTURE_MIPS32          4

#define MULTIBOOT2_TAG_TYPE_END                 0
#define MULTIBOOT2_TAG_TYPE_INFORMATION         1
#define MULTIBOOT2_TAG_TYPE_ADDRESS             2
#define MULTIBOOT2_TAG_TYPE_ENTRY_ADDRESS       3
#define MULTIBOOT2_TAG_TYPE_FLAGS               4
#define MULTIBOOT2_TAG_TYPE_FRAMEBUFFER         5
#define MULTIBOOT2_TAG_TYPE_MODULE_ALIGN        6
#define MULTIBOOT2_TAG_TYPE_EFI_BOOT_SERVICES   7
#define MULTIBOOT2_TAG_TYPE_ENTRY_ADDRESS_EFI32 8
#define MULTIBOOT2_TAG_TYPE_ENTRY_ADDRESS_EFI64 9
#define MULTIBOOT2_TAG_TYPE_RELOCATABLE         10

#define MULTIBOOT2_TAG_BOOT_CMD_LINE            1
#define MULTIBOOT2_TAG_BOOT_LOADER_NAME         2
#define MULTIBOOT2_TAG_MODULE                   3
#define MULTIBOOT2_TAG_BASIC_MEMINFO            4
#define MULTIBOOT2_TAG_BOOTDEV                  5
#define MULTIBOOT2_TAG_MMAP                     6
#define MULTIBOOT2_TAG_VBE                      7
#define MULTIBOOT2_TAG_FRAMEBUFFER              8
#define MULTIBOOT2_TAG_ELF_SECTIONS             9
#define MULTIBOOT2_TAG_APM                      10
#define MULTIBOOT2_TAG_EFI32                    11
#define MULTIBOOT2_TAG_EFI64                    12
#define MULTIBOOT2_TAG_SMBIOS                   13
#define MULTIBOOT2_TAG_ACPI_OLD                 14
#define MULTIBOOT2_TAG_ACPI_NEW                 15
#define MULTIBOOT2_TAG_NETWORK                  16
#define MULTIBOOT2_TAG_EFI_MMAP                 17
#define MULTIBOOT2_TAG_EFI_BS                   18
#define MULTIBOOT2_TAG_EFI32_IH                 19
#define MULTIBOOT2_TAG_EFI64_IH                 20
#define MULTIBOOT2_TAG_LOAD_BASE_ADDR           21

/// Multiboot2 information structure header (passed by the bootloader)
typedef struct {
    u32 total_size;
    u32 reserved;
    // Followed by tags
} __packed multiboot_info_t;

/// Common tag structure
typedef struct {
    u32 type;
    u32 size;
} __packed multiboot_tag_t;

/// Basic memory information tag
typedef struct {
    u32 type;
    u32 size;
    u32 mem_lower;
    u32 mem_upper;
} __packed multiboot_tag_basic_meminfo_t;

/// Memory map entry type constants (Multiboot2 spec §3.6.8)
#define MULTIBOOT2_MEMORY_AVAILABLE        1 ///< Usable RAM
#define MULTIBOOT2_MEMORY_RESERVED         2 ///< Reserved — do not use
#define MULTIBOOT2_MEMORY_ACPI_RECLAIMABLE 3 ///< ACPI tables — usable after ACPI init
#define MULTIBOOT2_MEMORY_NVS              4 ///< ACPI NVS — must be preserved
#define MULTIBOOT2_MEMORY_BADRAM           5 ///< Defective memory

/// Memory map entry
typedef struct {
    u64 addr;
    u64 len;
    u32 type;
    u32 reserved;
} __packed multiboot_mmap_entry_t;

/// Memory map tag
typedef struct {
    u32 type;
    u32 size;
    u32 entry_size;
    u32 entry_version;
    multiboot_mmap_entry_t entries[];
} __packed multiboot_tag_mmap_t;

/// Framebuffer type constants (fb_type field)
#define MULTIBOOT2_FRAMEBUFFER_TYPE_INDEXED  0
#define MULTIBOOT2_FRAMEBUFFER_TYPE_RGB      1
#define MULTIBOOT2_FRAMEBUFFER_TYPE_EGA_TEXT 2

/// Framebuffer tag
typedef struct {
    u32 type;
    u32 size;
    u64 addr;
    u32 pitch;
    u32 width;
    u32 height;
    u8 bpp;
    u8 fb_type;
    u8 reserved;
    // Color info — only valid when the type of the framebuffer (fb_type() is MULTIBOOT2_FRAMEBUFFER_TYPE_RGB
    u8 red_field_position;
    u8 red_mask_size;
    u8 green_field_position;
    u8 green_mask_size;
    u8 blue_field_position;
    u8 blue_mask_size;
} __packed multiboot_tag_framebuffer_t;

/// Boot command line tag
typedef struct {
    u32 type;
    u32 size;
    char string[];
} __packed multiboot_tag_string_t;

/// Module tag
typedef struct {
    u32 type;
    u32 size;
    u32 mod_start;
    u32 mod_end;
    char cmdline[];
} __packed multiboot_tag_module_t;

/// Get the first tag from multiboot info
static __always_inline __pure multiboot_tag_t * multiboot_first_tag(multiboot_info_t * info)
{
    return (multiboot_tag_t *) ((u8 *) info + 8);
}

/// Get the next tag (8-byte aligned)
static __always_inline __pure multiboot_tag_t * multiboot_next_tag(multiboot_tag_t * tag)
{
    u32 size = (tag->size + 7) & ~7u;
    return (multiboot_tag_t *) ((u8 *) tag + size);
}

/// Check if tag is the end tag
static __always_inline __pure bool multiboot_is_end_tag(multiboot_tag_t * tag)
{
    return tag->type == MULTIBOOT2_TAG_TYPE_END && tag->size == 8;
}

#endif /* JANUS_MULTIBOOT2_PROTOCOL_H */
