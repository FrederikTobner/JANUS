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

#ifndef BOOT_MEMMAP_H
#define BOOT_MEMMAP_H

/// @file memmap.h
/// @brief Protocol-agnostic physical memory map
///
/// Defines the canonical memory region types and the boot-time memory map
/// structure passed from a boot protocol implementation to the kernel.
/// All boot protocols translate their native types into these values.

#include <janus/types.h>

/// @brief Maximum number of memory map entries
///
/// 128 entries is sufficient for all real-world machines. Both Limine and
/// Multiboot2 produce far fewer entries in practice.
#define BOOT_MEMMAP_MAX_ENTRIES 128

/// @brief Type of a physical memory region
///
/// Values mirror the Limine memory map types for directness; Multiboot2
/// types are translated at parse time.
typedef enum {
    MEM_REGION_USABLE = 0,             ///< General-purpose usable RAM
    MEM_REGION_RESERVED,               ///< Reserved — must not be touched
    MEM_REGION_ACPI_RECLAIMABLE,       ///< ACPI tables — usable after ACPI init
    MEM_REGION_ACPI_NVS,               ///< ACPI NVS — must be preserved
    MEM_REGION_BAD,                    ///< Bad memory — hardware defect
    MEM_REGION_BOOTLOADER_RECLAIMABLE, ///< Bootloader data — usable after boot init
    MEM_REGION_KERNEL_AND_MODULES,     ///< Kernel image and loaded modules
    MEM_REGION_FRAMEBUFFER,            ///< Framebuffer memory
} mem_region_type_t;

/// @brief A single physical memory region
typedef struct {
    phys_addr_t base;       ///< Physical start address (page-aligned)
    u64 length;             ///< Length in bytes (page-aligned)
    mem_region_type_t type; ///< Region type
} mem_region_t;

/// @brief Physical memory map populated at boot time
typedef struct {
    mem_region_t entries[BOOT_MEMMAP_MAX_ENTRIES]; ///< Region array
    u32 count;                                     ///< Number of valid entries
} boot_memmap_t;

#endif /* BOOT_MEMMAP_H */
