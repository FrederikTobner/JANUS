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

#ifndef KMAIN_KERNEL_DESCRIPTOR_H
#define KMAIN_KERNEL_DESCRIPTOR_H

/**
 * @file kernel_descriptor.h
 * @brief Kernel descriptor — aggregates all per-kernel singleton state
 *
 * Built once during boot initialization in kernel_main, then passed as
 * const pointer through the entire kernel. Immutable after init.
 *
 * This type is kmain-private. Subsystems receive their own slice
 * (e.g., boot_context_t const *) — they never see kernel_descriptor_t.
 *
 * Future fields: memory_map, acpi_info, cpu_topology, ...
 */

#include <boot/context.h>

typedef struct kernel_descriptor {
    boot_context_t boot; /**< Boot protocol information */
} kernel_descriptor_t;

#endif /* KMAIN_KERNEL_DESCRIPTOR_H */
