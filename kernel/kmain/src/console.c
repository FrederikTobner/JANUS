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
#include <janus/errno.h>

static bool g_serial_active = false;
static bool g_tty_active = false;

static __cold bool init_serial(boot_context_t const * boot_context);
static __cold bool init_tty(boot_context_t const * boot_context, bool serial_available);
static void console_putc(char c, __unused void * ctx);

// Kernel printf wrapper
s32 kprintf(char const * fmtstr, ...)
{
    va_list ap;
    va_start(ap, fmtstr);
    s32 ret = vfmt_to(console_putc, NULL, fmtstr, ap);
    va_end(ap);
    return ret;
}

s32 vkprintf(char const * fmtstr, va_list ap)
{
    return vfmt_to(console_putc, NULL, fmtstr, ap);
}

__cold void console_init_early(void)
{
    if (g_serial_active) {
        return; // Already initialized — no-op
    }
    // Read address-translation parameters directly from the boot-protocol data
    // structures. On Limine (both x86_64 and AArch64) these are populated by
    // the bootloader before kernel_main is entered. On Multiboot2 all three
    // are 0, which is correct (x86_64 serial uses port I/O and ignores them).
    u64 hhdm_offset;
    phys_addr_t kernel_phys_base;
    virt_addr_t kernel_virt_base;
    boot_early_params(&hhdm_offset, &kernel_phys_base, &kernel_virt_base);
    if (drivers_serial_init(hhdm_offset, kernel_phys_base, kernel_virt_base) == JANUS_OK) {
        g_serial_active = true;
    }
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
        drivers_tty_set_color(TTY_COLOR_GREEN, TTY_COLOR_BLACK);
    }
}

static __cold bool init_serial(boot_context_t const * boot_context)
{
    if (g_serial_active) {
        // Already initialized by console_init_early() — skip re-init to
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

static __cold bool init_tty(boot_context_t const * boot_context, bool serial_available)
{
    if (boot_context->display.mode == DISPLAY_MODE_VGA_TEXT ||
        (boot_context->display.mode == DISPLAY_MODE_FRAMEBUFFER)) {
        if (drivers_tty_init(&(boot_context->display)) != JANUS_OK) {
            return false;
        }

        if (serial_available) {
            drivers_serial_puts("TTY driver initialized\n");
        }
        return true;
    }
    if (serial_available) {
        drivers_serial_puts("No display available, skipping TTY initialization\n");
    }
    return false;
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
