/*****************************************************************************
 * Copyright (C) 2025 by Frederik Tobner                                     *
 *                                                                           *
 * This file is part of TinyOS.                                              *
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

#ifndef BOOT_MULTIBOOT_H
#define BOOT_MULTIBOOT_H

/**
 * @file multiboot.h
 * @brief Multiboot2 protocol definitions
 *
 * Defines structures and constants for the Multiboot2 boot protocol.
 * See: https://www.gnu.org/software/grub/manual/multiboot2/multiboot.html
 */

#include <compiler.h>
#include <stdbool.h>
#include <tinyos/types.h>

// Multiboot2 magic value passed by bootloader in EAX
#define MULTIBOOT2_BOOTLOADER_MAGIC             0x36d76289

// Multiboot2 header magic (in the header itself)
#define MULTIBOOT2_HEADER_MAGIC                 0xe85250d6

// Architecture types
#define MULTIBOOT2_ARCHITECTURE_I386            0
#define MULTIBOOT2_ARCHITECTURE_MIPS32          4

// Tag types
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

// Multiboot2 information tag types (from bootloader)
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

/**
 * Multiboot2 information structure header
 * This is passed to the kernel by the bootloader
 */
struct multiboot_info {
    uint32_t total_size;
    uint32_t reserved;
    // Followed by tags
} __packed;

/**
 * Common tag structure
 */
struct multiboot_tag {
    uint32_t type;
    uint32_t size;
} __packed;

/**
 * Basic memory information tag
 */
struct multiboot_tag_basic_meminfo {
    uint32_t type;
    uint32_t size;
    uint32_t mem_lower; // Amount of lower memory in KB
    uint32_t mem_upper; // Amount of upper memory in KB
} __packed;

/**
 * Memory map entry
 */
struct multiboot_mmap_entry {
    uint64_t addr;
    uint64_t len;
    uint32_t type;
    uint32_t reserved;
} __packed;

/**
 * Memory map tag
 */
struct multiboot_tag_mmap {
    uint32_t type;
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
    struct multiboot_mmap_entry entries[];
} __packed;

/**
 * Boot command line tag
 */
struct multiboot_tag_string {
    uint32_t type;
    uint32_t size;
    char string[];
} __packed;

/**
 * Module tag
 */
struct multiboot_tag_module {
    uint32_t type;
    uint32_t size;
    uint32_t mod_start;
    uint32_t mod_end;
    char cmdline[];
} __packed;

/**
 * Get the first tag from multiboot info
 * @param info Pointer to multiboot info structure
 * @return Pointer to first tag
 */
static inline struct multiboot_tag * multiboot_first_tag(struct multiboot_info * info)
{
    return (struct multiboot_tag *) ((uint8_t *) info + 8);
}

/**
 * Get the next tag
 * @param tag Current tag
 * @return Pointer to next tag
 */
static inline struct multiboot_tag * multiboot_next_tag(struct multiboot_tag * tag)
{
    // Tags are 8-byte aligned
    uint32_t size = (tag->size + 7) & ~7;
    return (struct multiboot_tag *) ((uint8_t *) tag + size);
}

/**
 * Check if tag is the end tag
 * @param tag Tag to check
 * @return true if end tag, false otherwise
 */
static inline bool multiboot_is_end_tag(struct multiboot_tag * tag)
{
    return tag->type == MULTIBOOT2_TAG_TYPE_END && tag->size == 8;
}

#endif /* BOOT_MULTIBOOT_H */
