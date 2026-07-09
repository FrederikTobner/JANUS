/*****************************************************************************
 * Copyright (C) 2025 by Frederik Tobner                                     *
 *                                                                           *
 * This file is part of JANUS.                                               *
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

#ifndef CONTRACTS_DISPLAY_H
#define CONTRACTS_DISPLAY_H

/// @file contracts/display.h
/// @brief Shared display configuration contract.
///
/// Protocol-agnostic display configuration produced by the boot subsystem
/// and consumed by the drivers subsystem.  Structurally identical to the
/// memmap contract: one producer, one consumer, build-enforced allowlist.

#include <janus/types.h>

/// @brief Display mode reported by the bootloader.
///
/// Distinguishes the three states a bootloader can leave us in:
/// no display at all, a linear RGB framebuffer, or VGA text hardware.
typedef enum {
    DISPLAY_MODE_NONE = 0,    ///< No display information provided
    DISPLAY_MODE_FRAMEBUFFER, ///< Linear RGB framebuffer available
    DISPLAY_MODE_VGA_TEXT,    ///< VGA text mode confirmed (Multiboot2 EGA text)
} display_mode;

/// @brief Display configuration.
///
/// Describes the display mode and, when in framebuffer mode, the linear
/// RGB framebuffer parameters.  The @c mode field replaces the old
/// convention of testing @c framebuffer for NULL.
typedef struct display_info {
    display_mode mode; ///< What kind of display the bootloader provided
    void * framebuffer;  ///< Linear framebuffer base address (NULL for VGA text mode)
    u64 width;           ///< Horizontal resolution in pixels
    u64 height;          ///< Vertical resolution in pixels
    u64 pitch;           ///< Bytes per scanline
    u16 bpp;             ///< Bits per pixel
    u8 red_mask_shift;   ///< Red component bit position
    u8 green_mask_shift; ///< Green component bit position
    u8 blue_mask_shift;  ///< Blue component bit position
} display_info_t;

#endif /* CONTRACTS_DISPLAY_H */
