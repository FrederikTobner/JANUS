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

#ifndef BOOT_LIMINE_PRIVATE_H
#define BOOT_LIMINE_PRIVATE_H

/**
 * @file limine.h
 * @brief Private Limine boot protocol definitions
 *
 * Internal definitions for the Limine boot protocol used only within
 * the boot subsystem. Public structures (framebuffer, etc.) are in
 * <boot/protocol/limine.h>.
 *
 * See: https://github.com/limine-bootloader/limine/blob/trunk/PROTOCOL.md
 */

#include <boot/protocol/limine.h>
#include <janus/attributes.h>
#include <janus/types.h>

/* -------------------------- Boot Protocol Magic -------------------------- */

/// Limine magic value passed by our entry point (identifies Limine boot)
/// This is "LIMINE\0\0" encoded as a u64
#define LIMINE_BOOTLOADER_MAGIC 0x4C494D494E450000ULL

/* -------------------------- Internal Structures -------------------------- */

/// Limine base revision (protocol version)
struct limine_base_revision {
    u64 magic[2];
    u64 revision;
};

/// Limine response header (common to all responses)
struct limine_response {
    u64 revision;
};

/// HHDM (Higher Half Direct Map) response
/// The HHDM offset is added to physical addresses to get virtual addresses
struct limine_hhdm_response {
    u64 revision;
    u64 offset;
};

/* -------------------------- Memory Map -------------------------- */

/// Memory map entry types
#define LIMINE_MEMMAP_USABLE                 0
#define LIMINE_MEMMAP_RESERVED               1
#define LIMINE_MEMMAP_ACPI_RECLAIMABLE       2
#define LIMINE_MEMMAP_ACPI_NVS               3
#define LIMINE_MEMMAP_BAD_MEMORY             4
#define LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE 5
#define LIMINE_MEMMAP_KERNEL_AND_MODULES     6
#define LIMINE_MEMMAP_FRAMEBUFFER            7

/// Memory map entry structure
struct limine_memmap_entry {
    u64 base;
    u64 length;
    u64 type;
};

/// Memory map response
struct limine_memmap_response {
    u64 revision;
    u64 entry_count;
    struct limine_memmap_entry ** entries;
};

#endif /* BOOT_LIMINE_PRIVATE_H */
