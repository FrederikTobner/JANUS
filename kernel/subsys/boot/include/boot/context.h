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

#ifndef BOOT_CONTEXT_H
#define BOOT_CONTEXT_H

/// @file context.h
/// @brief Boot context — protocol-agnostic boot information
///
/// Provides boot protocol types and the boot_init() entry point.
/// The boot_context_t struct is public (per Coding-Style.md) —
/// consumers access fields directly.

#include <contracts/display.h>
#include <contracts/memmap.h>
#include <janus/attributes.h>
#include <janus/types.h>

/// @brief Boot protocol identifier
typedef enum {
    BOOT_PROTOCOL_UNKNOWN = 0,
    BOOT_PROTOCOL_MULTIBOOT2,
    BOOT_PROTOCOL_LIMINE,
} boot_protocol;

/// @brief Boot context — populated once during init, immutable after
///
/// Contains all boot-protocol-derived information needed by the kernel.
/// Public struct per Coding-Style.md: consumers access fields directly.
typedef struct {
    boot_protocol protocol;       ///< Which boot protocol was used
    u64 hhdm_offset;              ///< Higher Half Direct Map offset (0 for identity-mapped)
    phys_addr_t kernel_phys_base; ///< Kernel physical base address
    virt_addr_t kernel_virt_base; ///< Kernel virtual base address
    display_info_t display;       ///< Display config (check display.mode for availability)
    phys_addr_t kernel_phys_end;  ///< Physical end address of kernel image
    boot_memmap_t memmap;         ///< Physical memory map
} boot_context_t;

/// @brief Initialize boot context from the active boot protocol
///
/// Each protocol library (boot_limine, boot_multiboot2) provides its own
/// implementation of this symbol. Only one is linked per kernel binary.
/// The implementation must set every field unconditionally.
///
/// @param boot_context Boot context to populate
/// @return JANUS_OK on success, or a negative JANUS_E* error code on failure
error_t boot_init(boot_context_t * boot_context);

/// @brief Query address-translation parameters before boot_init completes.
///
/// Safe to call at any time — reads directly from boot-protocol data
/// structures populated by the bootloader before kernel_main is entered.
/// On Multiboot2 all outputs are 0 (x86_64 serial uses port I/O, 0 is valid).
///
/// @param hhdm_offset      Higher Half Direct Map offset.
/// @param kernel_phys_base Physical base address of the kernel image.
/// @param kernel_virt_base Virtual base address of the kernel image.
__cold void boot_early_params(u64 * hhdm_offset, phys_addr_t * kernel_phys_base, virt_addr_t * kernel_virt_base);

#endif /* BOOT_CONTEXT_H */
