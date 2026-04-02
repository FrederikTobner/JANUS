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

#include <kmain/console.h>

#include <boot/context.h>
#include <drivers/serial.h>
#include <drivers/tty.h>
#include <fmt/output.h>
#include <janus/attributes.h>

static bool g_serial_active = false;
static bool g_tty_active = false;

static __cold bool init_serial(boot_context_t const * boot_context)
{
    if (drivers_serial_init(
            boot_context->hhdm_offset, boot_context->kernel_phys_base, boot_context->kernel_virt_base) != 0) {
        return false;
    }

    drivers_serial_puts("Serial driver initialized\n");
    return true;
}

static __cold bool init_tty(boot_context_t const * boot_context, bool serial_available)
{
    display_info_t const * display = NULL;

    switch (boot_context->display_mode) {

    case BOOT_DISPLAY_FRAMEBUFFER:
        display = &boot_context->display;
        break;

    case BOOT_DISPLAY_VGA_TEXT:
        // Pass non-NULL with framebuffer == NULL → TTY selects VGA text mode
        display = &boot_context->display;
        break;

    case BOOT_DISPLAY_NONE:
        if (serial_available) {
            drivers_serial_puts("TTY skipped (no display available)\n");
        }
        return false;
    }

    if (drivers_tty_init(display) != 0) {
        return false;
    }

    if (serial_available) {
        drivers_serial_puts("TTY driver initialized\n");
    }
    return true;
}

static void console_putc(char c, __unused void * ctx)
{
    if (g_serial_active) {
        drivers_serial_putc(c);
    }
    if (g_tty_active) {
        drivers_tty_putc(c);
    }
}

// Kernel printf wrapper
s32 kprintf(char const * fmtstr, ...)
{
    va_list ap;
    va_start(ap, fmtstr);
    s32 ret = vfmt_to(console_putc, NULL, fmtstr, ap);
    va_end(ap);
    return ret;
}

// After serial/tty init, set up the output sink:
// (This should be called after both drivers are initialized and their availability is known)
__cold void console_init(boot_context_t const * boot_context)
{
    // Initialize drivers
    bool serial_available = init_serial(boot_context);
    bool tty_available = init_tty(boot_context, serial_available);
    g_serial_active = serial_available;
    g_tty_active = tty_available;
    if (tty_available) {
        drivers_tty_set_color(2, 0); // Green on black for TTY
    }
}
