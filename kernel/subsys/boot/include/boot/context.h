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

/**
 * @file context.h
 * @brief Boot context — protocol-agnostic boot information
 *
 * Provides boot protocol types and the boot_init() entry point.
 * The boot_context_t struct is public (per Coding-Style.md) —
 * consumers access fields directly.
 */

#include <janus/attributes.h>
#include <janus/types.h>

/**
 * @brief Boot protocol identifier
 */
typedef enum {
    BOOT_PROTOCOL_UNKNOWN = 0,
    BOOT_PROTOCOL_MULTIBOOT2,
    BOOT_PROTOCOL_LIMINE,
} boot_protocol_t;

/**
 * @brief Display mode reported by the bootloader
 *
 * Distinguishes the three states a bootloader can leave us in:
 * no display at all, a linear RGB framebuffer, or VGA text hardware.
 */
typedef enum {
    BOOT_DISPLAY_NONE = 0,    /**< No display information provided */
    BOOT_DISPLAY_FRAMEBUFFER, /**< Linear RGB framebuffer available */
    BOOT_DISPLAY_VGA_TEXT,    /**< VGA text mode confirmed (Multiboot2 EGA text) */
} boot_display_mode_t;

/**
 * @brief Protocol-agnostic display information
 *
 * Normalized framebuffer data populated by the protocol implementation.
 */
typedef struct boot_display_info {
    u8 * framebuffer;    /**< Linear framebuffer base address */
    u64 width;           /**< Horizontal resolution in pixels */
    u64 height;          /**< Vertical resolution in pixels */
    u64 pitch;           /**< Bytes per scanline */
    u16 bpp;             /**< Bits per pixel */
    u8 red_mask_shift;   /**< Red component bit position */
    u8 green_mask_shift; /**< Green component bit position */
    u8 blue_mask_shift;  /**< Blue component bit position */
} boot_display_info_t;

/**
 * @brief Boot context — populated once during init, immutable after
 *
 * Contains all boot-protocol-derived information needed by the kernel.
 * Public struct per Coding-Style.md: consumers access fields directly.
 */
typedef struct boot_context {
    boot_protocol_t protocol;         /**< Which boot protocol was used */
    u64 hhdm_offset;                  /**< Higher Half Direct Map offset (0 for identity-mapped) */
    phys_addr_t kernel_phys_base;     /**< Kernel physical base address */
    virt_addr_t kernel_virt_base;     /**< Kernel virtual base address */
    boot_display_info_t display;      /**< Framebuffer info (valid only when display_mode == FRAMEBUFFER) */
    boot_display_mode_t display_mode; /**< What kind of display the bootloader provided */
} boot_context_t;

/**
 * @brief Initialize boot context from the active boot protocol
 *
 * Each protocol library (boot_limine, boot_multiboot2) provides its own
 * implementation of this symbol. Only one is linked per kernel binary.
 * The implementation must set every field unconditionally.
 *
 * @param ctx Boot context to populate
 * @return 0 on success, non-zero on failure
 */
int boot_init(boot_context_t * ctx);

/**
 * @brief Convert a kernel virtual address to physical
 */
static __always_inline phys_addr_t boot_kernel_virt_to_phys(boot_context_t const * ctx, virt_addr_t virtual_address)
{
    return virtual_address - ctx->kernel_virt_base + ctx->kernel_phys_base;
}

/**
 * @brief Convert a physical address to its HHDM virtual mapping
 */
static __always_inline void * boot_phys_to_virt(boot_context_t const * ctx, phys_addr_t physical_address)
{
    return (void *) (physical_address + ctx->hhdm_offset);
}

#endif /* BOOT_CONTEXT_H */
