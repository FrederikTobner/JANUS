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
 * @file limine_requests.c
 * @brief Limine Boot Protocol Requests (Portable C Implementation)
 *
 * Limine request structures placed in dedicated sections that Limine
 * scans at boot time. This file is portable across architectures
 * (x86_64, AArch64) since it uses standard C with section attributes.
 *
 * See: https://github.com/limine-bootloader/limine/blob/trunk/PROTOCOL.md
 */

#include <janus/types.h>

/* ========================== Limine Protocol Constants ========================== */

/* Limine common magic (first 2 qwords of every request ID) */
#define LIMINE_COMMON_MAGIC_0 0xc7b1dd30df4c8b88ULL
#define LIMINE_COMMON_MAGIC_1 0x0a82e883a194f07bULL

/* Request start/end markers */
#define LIMINE_REQUESTS_START_MARKER \
    {0xf6b8f4b39de7d1aeULL, 0xfab91a6940fcb9cfULL, 0x785c6ed015d3e316ULL, 0x181e920a7852b9d9ULL}

#define LIMINE_REQUESTS_END_MARKER \
    {0xadc0e0531bb10d03ULL, 0x9572709f31764c62ULL, 0xa330a8b4e59a6668ULL, 0xc5bfb1ba7e9fcc7fULL}

/* Base revision magic */
#define LIMINE_BASE_REVISION_MAGIC_0 0xf9562b2d5c95a6c8ULL
#define LIMINE_BASE_REVISION_MAGIC_1 0x6a7b384944536bdcULL

/* Request ID macros */
#define LIMINE_ENTRY_POINT_REQUEST_ID \
    {LIMINE_COMMON_MAGIC_0, LIMINE_COMMON_MAGIC_1, 0x13d86c035a1cd3e1ULL, 0x2b0caa89d8f3026aULL}

#define LIMINE_STACK_SIZE_REQUEST_ID \
    {LIMINE_COMMON_MAGIC_0, LIMINE_COMMON_MAGIC_1, 0x224ef0460a8e8926ULL, 0xe1cb0fc25f46ea3dULL}

#define LIMINE_HHDM_REQUEST_ID \
    {LIMINE_COMMON_MAGIC_0, LIMINE_COMMON_MAGIC_1, 0x48dcf1cb8ad2b852ULL, 0x63984e959a98244bULL}

#define LIMINE_FRAMEBUFFER_REQUEST_ID \
    {LIMINE_COMMON_MAGIC_0, LIMINE_COMMON_MAGIC_1, 0x9d5827dcd881dd75ULL, 0xa3148604f6fab11bULL}

/* ========================== Request Structures ========================== */

/* Entry point request */
struct limine_entry_point_request {
    u64 id[4];
    u64 revision;
    void * response;
    void (*entry)(void);
};

/* Stack size request */
struct limine_stack_size_request {
    u64 id[4];
    u64 revision;
    void * response;
    u64 stack_size;
};

/* HHDM request */
struct limine_hhdm_request {
    u64 id[4];
    u64 revision;
    void * response;
};

/* Framebuffer request */
struct limine_framebuffer_request {
    u64 id[4];
    u64 revision;
    void * response;
};

/* ========================== External Entry Point ========================== */

/* Assembly entry point that sets up arguments and calls kernel_main */
extern void _start_limine(void);

/* ========================== Section Markers ========================== */

__attribute__((used, section(".limine_requests_start"))) static volatile u64 limine_requests_start_marker[4] =
    LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end"))) static volatile u64 limine_requests_end_marker[4] =
    LIMINE_REQUESTS_END_MARKER;

/* ========================== Limine Requests ========================== */

/*
 * Base revision - tells Limine which protocol revision we support.
 * Revision 3 is for Limine v8+.
 */
__attribute__((used, section(".limine_requests"))) static volatile u64 limine_base_revision[3] = {
    LIMINE_BASE_REVISION_MAGIC_0, LIMINE_BASE_REVISION_MAGIC_1, 3 /* Revision 3 (Limine v8+) */
};

/*
 * Entry point request - tells Limine where to jump after boot.
 */
__attribute__((
    used, section(".limine_requests"))) static volatile struct limine_entry_point_request limine_entry_point_request = {
    .id = LIMINE_ENTRY_POINT_REQUEST_ID,
    .revision = 0,
    .response = NULL,
    .entry = _start_limine,
};

/*
 * Stack size request - requests a 64KB stack from Limine.
 */
__attribute__((
    used, section(".limine_requests"))) static volatile struct limine_stack_size_request limine_stack_size_request = {
    .id = LIMINE_STACK_SIZE_REQUEST_ID,
    .revision = 0,
    .response = NULL,
    .stack_size = 65536, /* 64KB */
};

/*
 * HHDM (Higher Half Direct Map) request.
 * Required to access physical memory via virtual addresses.
 * The response contains the HHDM offset to add to physical addresses.
 */
__attribute__((used, section(".limine_requests"))) volatile struct limine_hhdm_request limine_hhdm_request = {
    .id = LIMINE_HHDM_REQUEST_ID,
    .revision = 0,
    .response = NULL,
};

/*
 * Framebuffer request - requests a graphical framebuffer.
 * Used by the TTY driver for graphical output.
 */
__attribute__((used,
               section(".limine_requests"))) volatile struct limine_framebuffer_request limine_framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0,
    .response = NULL,
};
