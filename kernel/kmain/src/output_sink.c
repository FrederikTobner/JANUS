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

/// @file kmain/output_sink.c
/// @brief Kernel output sink — fans kio output to serial and console.

#include <kmain/output_sink.h>

#include <boot/context.h>
#include <drivers/console.h>
#include <drivers/serial.h>
#include <janus/attributes.h>
#include <janus/errno.h>
#include <kio/output.h>

static bool g_serial_active = false;
static bool g_console_active = false;

static __cold bool init_serial(boot_context_t const * boot_context);
static __cold bool init_console(boot_context_t const * boot_context, bool serial_available);
static void output_sink_putc(char c);

__cold void output_sink_init_early(void)
{
    if (g_serial_active) {
        return; // Already initialized
    }
    // Read address-translation parameters directly from boot-protocol data
    // structures.  On Limine these are populated before kernel_main is entered.
    // On Multiboot2 all three are 0 (x86_64 serial uses port I/O; zeros are valid).
    u64 hhdm_offset;
    phys_addr_t kernel_phys_base;
    virt_addr_t kernel_virt_base;
    boot_early_params(&hhdm_offset, &kernel_phys_base, &kernel_virt_base);
    if (drivers_serial_init(hhdm_offset, kernel_phys_base, kernel_virt_base) == JANUS_OK) {
        g_serial_active = true;
        kio_register_putc(output_sink_putc);
    }
}

__cold void output_sink_init(boot_context_t const * boot_context)
{
    bool serial_available = init_serial(boot_context);
    bool console_available = init_console(boot_context, serial_available);
    g_serial_active = serial_available;
    g_console_active = console_available;
    kio_register_putc(output_sink_putc);
    if (console_available) {
        drivers_console_set_color(CONSOLE_COLOR_GREEN, CONSOLE_COLOR_BLACK);
    }
}

static __cold bool init_serial(boot_context_t const * boot_context)
{
    if (g_serial_active) {
        // Already initialized by output_sink_init_early() — skip re-init to
        // avoid resetting driver state (e.g. the AArch64 MMU pool index).
        return true;
    }
    if (drivers_serial_init(
            boot_context->hhdm_offset, boot_context->kernel_phys_base, boot_context->kernel_virt_base) != JANUS_OK) {
        return false;
    }
    drivers_serial_puts("Serial driver initialized\n");
    return true;
}

static __cold bool init_console(boot_context_t const * boot_context, bool serial_available)
{
    if (boot_context->display.mode == DISPLAY_MODE_VGA_TEXT || boot_context->display.mode == DISPLAY_MODE_FRAMEBUFFER) {
        if (drivers_console_init(&boot_context->display) != JANUS_OK) {
            return false;
        }
        if (serial_available) {
            drivers_serial_puts("Console driver initialized\n");
        }
        return true;
    }
    if (serial_available) {
        drivers_serial_puts("No display available, skipping console initialization\n");
    }
    return false;
}

static void output_sink_putc(char c)
{
    if (g_serial_active) {
        drivers_serial_putc(c);
    }
    if (g_console_active) {
        drivers_console_putc(c);
    }
}
