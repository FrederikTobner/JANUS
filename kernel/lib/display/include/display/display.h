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

#ifndef DISPLAY_DISPLAY_H
#define DISPLAY_DISPLAY_H

/**
 * @file display.h
 * @brief Shared display information type
 *
 * Protocol-agnostic display configuration shared between the boot
 * subsystem (which populates it) and the drivers subsystem (which
 * consumes it).  Lives in lib/ so both subsystems can depend on it
 * without depending on each other.
 */

#include <janus/types.h>

/**
 * @brief Display configuration
 *
 * Describes a linear RGB framebuffer.  When passed as a pointer:
 * - NULL means no display is available
 * - Non-NULL with framebuffer == NULL means VGA text mode (x86_64 only)
 * - Non-NULL with framebuffer != NULL means framebuffer mode
 */
typedef struct display_info {
    void * framebuffer;  /**< Linear framebuffer base address (NULL for VGA text mode) */
    u64 width;           /**< Horizontal resolution in pixels */
    u64 height;          /**< Vertical resolution in pixels */
    u64 pitch;           /**< Bytes per scanline */
    u16 bpp;             /**< Bits per pixel */
    u8 red_mask_shift;   /**< Red component bit position */
    u8 green_mask_shift; /**< Green component bit position */
    u8 blue_mask_shift;  /**< Blue component bit position */
} display_info_t;

#endif /* DISPLAY_DISPLAY_H */
