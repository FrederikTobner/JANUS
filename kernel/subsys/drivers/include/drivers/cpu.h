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
 * @file cpu.h
 * @brief CPU control interface.
 *
 * Header-only driver. Architecture implementation is pulled in via
 * include path resolution: <arch/drivers/cpu.h> → <arch/impl/drivers/cpu.h>
 */

#ifndef DRIVERS_CPU_H
#define DRIVERS_CPU_H

#include <arch/drivers/cpu.h>
#include <janus/attributes.h>

static __always_inline void drivers_cpu_halt(void)
{
    arch_cpu_halt();
}

static __always_inline void drivers_cpu_disable_interrupts(void)
{
    arch_cpu_disable_interrupts();
}

static __always_inline void drivers_cpu_enable_interrupts(void)
{
    arch_cpu_enable_interrupts();
}
/**
 * @brief Disable interrupts and halt forever.
 *
 * This function never returns and is used for unrecoverable errors.
 */
static __always_inline __noreturn void drivers_cpu_halt_forever(void)
{
    arch_cpu_disable_interrupts();
    for (;;) {
        arch_cpu_halt();
    }
}


#endif /* DRIVERS_CPU_H */
