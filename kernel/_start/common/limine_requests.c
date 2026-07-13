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

/// @file limine_requests.c
/// @brief Limine Boot Protocol Requests (Portable C Implementation)
///
/// Limine request structures placed in dedicated sections that Limine
/// scans at boot time. This file is portable across architectures
/// (x86_64, AArch64) since it uses standard C with section attributes.
///
/// See: https://github.com/limine-bootloader/limine/blob/trunk/PROTOCOL.md

#include "limine_protocol.h"

#include <janus/attributes.h>

// Assembly entry point that sets up arguments and calls kernel_main
extern void _start_limine(void);

__used __section(".limine_requests_start") static volatile u64 limine_requests_start_marker[4] =
    LIMINE_REQUESTS_START_MARKER;

__used __section(".limine_requests_end") static volatile u64 limine_requests_end_marker[4] = LIMINE_REQUESTS_END_MARKER;

// Base revision - tells Limine which protocol revision we support.
// Revision 3 is for Limine v8+.
__used __section(".limine_requests") static volatile u64 limine_base_revision[3] = {
    LIMINE_BASE_REVISION_MAGIC_0, LIMINE_BASE_REVISION_MAGIC_1, 3 // Revision 3 (Limine v8+)
};

// Entry point request - tells Limine where to jump after boot.
__used __section(".limine_requests") static volatile limine_entry_point_request_t limine_entry_point_request = {
    .id = LIMINE_ENTRY_POINT_REQUEST_ID,
    .revision = 0,
    .response = NULL,
    .entry = _start_limine,
};

// Stack size request - requests a 64KB stack from Limine.
__used __section(".limine_requests") static volatile limine_stack_size_request_t limine_stack_size_request = {
    .id = LIMINE_STACK_SIZE_REQUEST_ID,
    .revision = 0,
    .response = NULL,
    .stack_size = 65536, // 64KB
};

// HHDM (Higher Half Direct Map) request.
// Required to access physical memory via virtual addresses.
// The response contains the HHDM offset to add to physical addresses.
__used __section(".limine_requests") volatile limine_hhdm_request_t limine_hhdm_request = {
    .id = LIMINE_HHDM_REQUEST_ID,
    .revision = 0,
    .response = NULL,
};

// Framebuffer request - requests a graphical framebuffer.
// Used by the TTY driver for graphical output.
__used __section(".limine_requests") volatile limine_framebuffer_request_t limine_framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0,
    .response = NULL,
};

// Executable address request - provides kernel physical and virtual base addresses.
// Needed for aarch64 to compute physical addresses of kernel memory for page tables.
__used __section(".limine_requests") volatile limine_executable_address_request_t limine_executable_address_request = {
    .id = LIMINE_EXECUTABLE_ADDRESS_REQUEST_ID,
    .revision = 0,
    .response = NULL,
};

// Memory map request - provides the physical memory map from the bootloader.
// Required by the PMM to discover usable RAM regions.
__used __section(".limine_requests") volatile limine_memmap_request_t limine_memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0,
    .response = NULL,
};
