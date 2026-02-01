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

#ifndef BOOT_PROTOCOL_LIMINE_H
#define BOOT_PROTOCOL_LIMINE_H

/**
 * @file limine.h
 * @brief Public Limine boot protocol definitions
 *
 * Defines structures for Limine protocol data that other subsystems
 * may need to access (e.g., framebuffer info for TTY driver).
 *
 * See: https://github.com/limine-bootloader/limine/blob/trunk/PROTOCOL.md
 */

#include <janus/types.h>

/* -------------------------- Framebuffer Structures -------------------------- */

/// Framebuffer memory model types
#define LIMINE_FRAMEBUFFER_RGB 1

/**
 * @brief Limine video mode structure
 */
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

/**
 * @brief Limine framebuffer structure
 *
 * Describes a single framebuffer provided by the bootloader.
 */
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
    /* Revision 1+ */
    u64 mode_count;
    struct limine_video_mode ** modes;
};

/**
 * @brief Limine framebuffer response structure
 *
 * Contains an array of framebuffers provided by the bootloader.
 */
struct limine_framebuffer_response {
    u64 revision;
    u64 framebuffer_count;
    struct limine_framebuffer ** framebuffers;
};

#endif /* BOOT_PROTOCOL_LIMINE_H */
