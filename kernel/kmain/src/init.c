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
 * Used when the bootloader confirms VGA text hardware is present
 * (Multiboot2 EGA text mode).  The VGA buffer at 0xB8000 must be
 * directly accessible (identity-mapped).
 *
 * Only reachable on x86_64 — the Multiboot2 protocol library is
 * never linked on aarch64, so BOOT_DISPLAY_VGA_TEXT cannot appear.
 *
 * @param serial_available Whether serial output is available for logging
 * @return true if TTY was initialized successfully
 */
static __cold bool init_tty_vga(bool serial_available)
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
 * Used when the bootloader provides a graphical framebuffer (Limine,
 * or Multiboot2 with a framebuffer header tag).  The framebuffer
 * address must already be accessible (identity-mapped or via HHDM).
 *
 * @param display Boot display info from the boot context
 * @param serial_available Whether serial output is available for logging
 * @return true if TTY was initialized successfully
 */
static __cold bool init_tty_framebuffer(boot_display_info_t const * display, bool serial_available)
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

__cold bool kinit_serial(boot_context_t const * boot_context)
{
    if (drivers_serial_init(
            boot_context->hhdm_offset, boot_context->kernel_phys_base, boot_context->kernel_virt_base) != 0) {
        return false;
    }

    drivers_serial_puts("Serial driver initialized\n");
    return true;
}

__cold bool kinit_tty(boot_context_t const * boot_context, bool serial_available)
{
    switch (boot_context->display_mode) {

    case BOOT_DISPLAY_FRAMEBUFFER:
        return init_tty_framebuffer(&boot_context->display, serial_available);

    case BOOT_DISPLAY_VGA_TEXT:
        return init_tty_vga(serial_available);

    case BOOT_DISPLAY_NONE:
        if (serial_available) {
            drivers_serial_puts("TTY skipped (no display available)\n");
        }
        return false;
    }

    return false;
}
