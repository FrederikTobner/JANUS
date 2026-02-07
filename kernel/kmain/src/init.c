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

/**
 * @file init.c
 * @brief Kernel initialization routines.
 *
 * Handles early kernel initialization including driver setup.
 */

#include <kmain/init.h>

#include <boot/protocol/limine.h>
#include <drivers/serial.h>
#include <drivers/tty.h>

/* -------------------------- Private Functions -------------------------- */

/**
 * @brief Initialize TTY with VGA text mode.
 *
 * Used for identity-mapped boot (Multiboot2) where VGA buffer is directly
 * accessible at 0xB8000.
 *
 * @param serial_available Whether serial output is available for logging
 * @return true if TTY was initialized successfully
 */
static bool init_tty_vga(bool serial_available)
{
    if (drivers_tty_init(NULL) != 0) {
        return false;
    }

    if (serial_available) {
        drivers_serial_puts("TTY driver initialized (VGA text mode)\n");
    }
    return true;
}

/**
 * @brief Initialize TTY with framebuffer mode.
 *
 * Used for higher-half boot (Limine) where we need to use the framebuffer
 * for text rendering since VGA memory isn't identity-mapped.
 *
 * @param fb Pointer to Limine framebuffer structure
 * @param serial_available Whether serial output is available for logging
 * @return true if TTY was initialized successfully
 */
static bool init_tty_framebuffer(struct limine_framebuffer const * fb, bool serial_available)
{
    tty_display_config_t config;
    config.framebuffer = (u8 *) fb->address;
    config.width = fb->width;
    config.height = fb->height;
    config.pitch = fb->pitch;
    config.bpp = fb->bpp;
    config.red_mask_shift = fb->red_mask_shift;
    config.green_mask_shift = fb->green_mask_shift;
    config.blue_mask_shift = fb->blue_mask_shift;

    if (drivers_tty_init(&config) != 0) {
        return false;
    }

    if (serial_available) {
        drivers_serial_puts("TTY driver initialized (framebuffer mode)\n");
    }
    return true;
}

bool kinit_serial(u64 hhdm_offset, u64 kernel_phys_base, u64 kernel_virt_base)
{
    if (drivers_serial_init(hhdm_offset, kernel_phys_base, kernel_virt_base) != 0) {
        return false;
    }

    drivers_serial_puts("Serial driver initialized\n");
    return true;
}

bool kinit_tty(u64 hhdm_offset, void * fb_info, bool serial_available)
{
    if (hhdm_offset == 0) {
        // Identity-mapped (Multiboot2): VGA buffer is directly accessible
        return init_tty_vga(serial_available);
    }

    // Higher-half mapped (Limine): Need framebuffer for text output
    if (fb_info == NULL) {
        if (serial_available) {
            drivers_serial_puts("TTY skipped (Limine without framebuffer)\n");
        }
        return false;
    }

    struct limine_framebuffer_response const * fb_resp = fb_info;
    if (fb_resp->framebuffer_count == 0 || fb_resp->framebuffers == NULL) {
        if (serial_available) {
            drivers_serial_puts("TTY skipped (no framebuffer available)\n");
        }
        return false;
    }

    return init_tty_framebuffer(fb_resp->framebuffers[0], serial_available);
}
