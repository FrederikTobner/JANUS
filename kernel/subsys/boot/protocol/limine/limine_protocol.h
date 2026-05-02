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

#ifndef JANUS_LIMINE_PROTOCOL_H
#define JANUS_LIMINE_PROTOCOL_H

/// @file limine_protocol.h
/// @brief Canonical Limine boot protocol type definitions
///
/// Unified header for all Limine protocol constants, request structures,
/// and response structures. Shared between limine_requests.c (request
/// instances) and boot_limine (response parsing).
///
/// See: https://github.com/limine-bootloader/limine/blob/trunk/PROTOCOL.md

#include <janus/types.h>

/// Common magic (first 2 qwords of every request ID)
#define LIMINE_COMMON_MAGIC_0 0xc7b1dd30df4c8b88ULL
#define LIMINE_COMMON_MAGIC_1 0x0a82e883a194f07bULL

// clang-format off
/// Request region markers (scanned by Limine in the ELF binary)
#define LIMINE_REQUESTS_START_MARKER \
    {0xf6b8f4b39de7d1aeULL, 0xfab91a6940fcb9cfULL, 0x785c6ed015d3e316ULL, 0x181e920a7852b9d9ULL}

#define LIMINE_REQUESTS_END_MARKER \
    {0xadc0e0531bb10d03ULL, 0x9572709f31764c62ULL, 0xa330a8b4e59a6668ULL, 0xc5bfb1ba7e9fcc7fULL}

/// Base revision magic
#define LIMINE_BASE_REVISION_MAGIC_0 0xf9562b2d5c95a6c8ULL
#define LIMINE_BASE_REVISION_MAGIC_1 0x6a7b384944536bdcULL

/// Bootloader magic ("LIMINE\0\0" as u64) — used for protocol identification
#define LIMINE_BOOTLOADER_MAGIC      0x4C494D494E450000ULL

#define LIMINE_ENTRY_POINT_REQUEST_ID \
    {LIMINE_COMMON_MAGIC_0, LIMINE_COMMON_MAGIC_1, 0x13d86c035a1cd3e1ULL, 0x2b0caa89d8f3026aULL}

#define LIMINE_STACK_SIZE_REQUEST_ID \
    {LIMINE_COMMON_MAGIC_0, LIMINE_COMMON_MAGIC_1, 0x224ef0460a8e8926ULL, 0xe1cb0fc25f46ea3dULL}

#define LIMINE_HHDM_REQUEST_ID \
    {LIMINE_COMMON_MAGIC_0, LIMINE_COMMON_MAGIC_1, 0x48dcf1cb8ad2b852ULL, 0x63984e959a98244bULL}

#define LIMINE_FRAMEBUFFER_REQUEST_ID \
    {LIMINE_COMMON_MAGIC_0, LIMINE_COMMON_MAGIC_1, 0x9d5827dcd881dd75ULL, 0xa3148604f6fab11bULL}

#define LIMINE_EXECUTABLE_ADDRESS_REQUEST_ID \
    {LIMINE_COMMON_MAGIC_0, LIMINE_COMMON_MAGIC_1, 0x71ba76863cc55f63ULL, 0xb2644a48c516a487ULL}
// clang-format on

/// Entry point request
struct limine_entry_point_request {
    u64 id[4];
    u64 revision;
    void * response;
    void (*entry)(void);
};

/// Stack size request
struct limine_stack_size_request {
    u64 id[4];
    u64 revision;
    void * response;
    u64 stack_size;
};

/// HHDM (Higher Half Direct Map) request
struct limine_hhdm_request {
    u64 id[4];
    u64 revision;
    void * response;
};

/// Framebuffer request
struct limine_framebuffer_request {
    u64 id[4];
    u64 revision;
    void * response;
};

/// Executable address request — provides kernel physical/virtual base
struct limine_executable_address_request {
    u64 id[4];
    u64 revision;
    void * response;
};

/// Base revision (protocol version)
struct limine_base_revision {
    u64 magic[2];
    u64 revision;
};

/// Generic response header (common to all responses)
struct limine_response {
    u64 revision;
};

/// HHDM response — the offset to add to physical addresses
struct limine_hhdm_response {
    u64 revision;
    u64 offset;
};

/// Executable address response — kernel load addresses
struct limine_executable_address_response {
    u64 revision;
    u64 physical_base;
    u64 virtual_base;
};

/// Framebuffer memory model types
#define LIMINE_FRAMEBUFFER_RGB 1

/// Video mode descriptor
struct limine_video_mode {
    u64 pitch;
    u64 width;
    u64 height;
    u16 bpp;
    u8 memory_model;
    u8 red_mask_size;
    u8 red_mask_shift;
    u8 green_mask_size;
    u8 green_mask_shift;
    u8 blue_mask_size;
    u8 blue_mask_shift;
};

/// Individual framebuffer descriptor
struct limine_framebuffer {
    void * address;
    u64 width;
    u64 height;
    u64 pitch;
    u16 bpp;
    u8 memory_model;
    u8 red_mask_size;
    u8 red_mask_shift;
    u8 green_mask_size;
    u8 green_mask_shift;
    u8 blue_mask_size;
    u8 blue_mask_shift;
    u8 unused[7];
    u64 edid_size;
    void * edid;
    // Revision 1+
    u64 mode_count;
    struct limine_video_mode ** modes;
};

/// Framebuffer response — array of framebuffers
struct limine_framebuffer_response {
    u64 revision;
    u64 framebuffer_count;
    struct limine_framebuffer ** framebuffers;
};

/// Memory map entry types
#define LIMINE_MEMMAP_USABLE                 0
#define LIMINE_MEMMAP_RESERVED               1
#define LIMINE_MEMMAP_ACPI_RECLAIMABLE       2
#define LIMINE_MEMMAP_ACPI_NVS               3
#define LIMINE_MEMMAP_BAD_MEMORY             4
#define LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE 5
#define LIMINE_MEMMAP_KERNEL_AND_MODULES     6
#define LIMINE_MEMMAP_FRAMEBUFFER            7

/// Memory map entry
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

#endif /* JANUS_LIMINE_PROTOCOL_H */
