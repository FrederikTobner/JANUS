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

#ifndef BOOT_INFO_H
#define BOOT_INFO_H

/**
 * @file info.h
 * @brief Boot information storage and retrieval
 *
 * Provides a unified interface to access boot information regardless
 * of the boot protocol used (Multiboot2, Limine, etc.)
 */

#include <janus/types.h>

/**
 * @brief Boot protocol types
 */
typedef enum {
    BOOT_PROTOCOL_UNKNOWN = 0,
    BOOT_PROTOCOL_MULTIBOOT2,
    BOOT_PROTOCOL_LIMINE,
} boot_protocol_t;

/**
 * @brief Initialize boot info from loader handoff
 *
 * Must be called early in kernel_main before drivers are initialized.
 *
 * @param loader_magic Magic value from bootloader
 * @param info Protocol-specific info pointer
 * @return 0 on success, negative error code on failure
 */
error_t boot_info_init(u64 loader_magic, void * info);

/**
 * @brief Get the boot protocol type
 *
 * @return The boot protocol used
 */
boot_protocol_t boot_info_get_protocol(void);

/**
 * @brief Get the Higher Half Direct Map offset
 *
 * Under Limine, physical addresses must be accessed through the HHDM.
 * Under Multiboot2 with our identity-mapped boot, this returns 0.
 *
 * To convert a physical address to a virtual address:
 *   virtual_addr = physical_addr + boot_info_get_hhdm_offset()
 *
 * @return The HHDM offset (0 for identity-mapped boot)
 */
u64 boot_info_get_hhdm_offset(void);

/**
 * @brief Get kernel physical base address
 *
 * Returns the physical address where the kernel was loaded.
 * Only valid under Limine boot protocol.
 *
 * @return Kernel physical base address, or 0 if unavailable
 */
u64 boot_info_get_kernel_phys_base(void);

/**
 * @brief Get kernel virtual base address
 *
 * Returns the virtual address where the kernel is mapped.
 * Only valid under Limine boot protocol.
 *
 * @return Kernel virtual base address, or 0 if unavailable
 */
u64 boot_info_get_kernel_virt_base(void);

/**
 * @brief Convert a kernel virtual address to physical
 *
 * Converts a kernel-space virtual address to its physical address.
 * This uses the kernel physical/virtual bases, NOT the HHDM.
 *
 * @param virt Kernel virtual address
 * @return Physical address
 */
static inline u64 boot_kernel_virt_to_phys(u64 virt)
{
    return virt - boot_info_get_kernel_virt_base() + boot_info_get_kernel_phys_base();
}

/**
 * @brief Convert a physical address to a virtual address
 *
 * Uses the HHDM offset to convert physical addresses to virtual.
 *
 * @param phys Physical address
 * @return Virtual address
 */
static inline void * boot_phys_to_virt(u64 phys)
{
    return (void *) (phys + boot_info_get_hhdm_offset());
}

#endif /* BOOT_INFO_H */
