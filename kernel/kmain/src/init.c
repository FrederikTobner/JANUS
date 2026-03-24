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
 * All boot protocol details are accessed through the public boot context.
 */

#include <kmain/init.h>

#include <boot/context.h>
#include <drivers/serial.h>
#include <drivers/tty.h>

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
 * @param display Boot display info from the boot context
 * @param serial_available Whether serial output is available for logging
 * @return true if TTY was initialized successfully
 */
static bool init_tty_framebuffer(boot_display_info_t const * display, bool serial_available)
{
    tty_display_config_t config;
    config.framebuffer = display->framebuffer;
    config.width = display->width;
    config.height = display->height;
    config.pitch = display->pitch;
    config.bpp = display->bpp;
    config.red_mask_shift = display->red_mask_shift;
    config.green_mask_shift = display->green_mask_shift;
    config.blue_mask_shift = display->blue_mask_shift;

    if (drivers_tty_init(&config) != 0) {
        return false;
    }

    if (serial_available) {
        drivers_serial_puts("TTY driver initialized (framebuffer mode)\n");
    }
    return true;
}

bool kinit_serial(boot_context_t const * boot_context)
{
    if (drivers_serial_init(boot_context->hhdm_offset, boot_context->kernel_phys_base, boot_context->kernel_virt_base) != 0) {
        return false;
    }

    drivers_serial_puts("Serial driver initialized\n");
    return true;
}

bool kinit_tty(boot_context_t const * boot_context, bool serial_available)
{
    if (boot_context->hhdm_offset == 0) {
        // Identity-mapped (Multiboot2): VGA buffer is directly accessible
        return init_tty_vga(serial_available);
    }

    // Higher-half mapped (Limine): Need framebuffer for text output
    if (!boot_context->has_display) {
        if (serial_available) {
            drivers_serial_puts("TTY skipped (no framebuffer available)\n");
        }
        return false;
    }

    return init_tty_framebuffer(&boot_context->display, serial_available);
}
