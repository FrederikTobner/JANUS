/*****************************************************************************
 * Copyright (C) 2025 by Frederik Tobner                                     *
 *                                                                           *
 * This file is part of JANUS.                                              *
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
 * @file limine_boot.c
 * @brief Limine boot protocol initialization
 *
 * Reads Limine request responses and populates the boot context.
 * Called from kernel_main via the common boot_init() symbol.
 */

#include "limine_protocol.h"

#include <boot/context.h>

// Extern request symbols (defined in limine_requests.c)

extern volatile struct limine_hhdm_request limine_hhdm_request;
extern volatile struct limine_framebuffer_request limine_framebuffer_request;
extern volatile struct limine_executable_address_request limine_executable_address_request;

/**
 * @brief Initialize the boot context from Limine responses
 *
 * Reads the response pointers from each Limine request structure,
 * validates them, and populates the boot context. Sets every field
 * unconditionally to avoid uninitialized reads.
 *
 * @param ctx Boot context to populate
 * @return 0 on success, non-zero on failure (missing required responses)
 */
int boot_init(boot_context_t * ctx)
{
    // Set safe defaults for ALL fields unconditionally
    ctx->protocol = BOOT_PROTOCOL_LIMINE;
    ctx->hhdm_offset = 0;
    ctx->kernel_phys_base = 0;
    ctx->kernel_virt_base = 0;
    ctx->has_display = false;

    // HHDM offset is required for address translation
    struct limine_hhdm_response const * hhdm = limine_hhdm_request.response;
    if (hhdm == NULL) {
        return -1;
    }
    ctx->hhdm_offset = hhdm->offset;

    // Executable address (required on aarch64, optional on x86_64)
    struct limine_executable_address_response const * exec_addr = limine_executable_address_request.response;
    if (exec_addr != NULL) {
        ctx->kernel_phys_base = exec_addr->physical_base;
        ctx->kernel_virt_base = exec_addr->virtual_base;
    }

    // Framebuffer is optional — if not present, has_display remains false
    struct limine_framebuffer_response const * fb_resp = limine_framebuffer_request.response;
    if (fb_resp != NULL && fb_resp->framebuffer_count > 0 && fb_resp->framebuffers != NULL) {
        struct limine_framebuffer const * fb = fb_resp->framebuffers[0];
        ctx->display = (boot_display_info_t) {
            .framebuffer = (u8 *) fb->address,
            .width = fb->width,
            .height = fb->height,
            .pitch = fb->pitch,
            .bpp = fb->bpp,
            .red_mask_shift = fb->red_mask_shift,
            .green_mask_shift = fb->green_mask_shift,
            .blue_mask_shift = fb->blue_mask_shift,
        };
        ctx->has_display = true;
    }

    return 0;
}
