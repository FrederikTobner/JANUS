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

/// @file arch/impl/kio/halt.h
/// @brief x86_64 kio halt implementation.

#ifndef X86_64_IMPL_KIO_HALT_H
#define X86_64_IMPL_KIO_HALT_H

#include <asm/cpu.h>
#include <janus/attributes.h>

static __always_inline __noreturn void arch_kio_halt_forever(void)
{
    asm_cpu_cli();
    for (;;) {
        asm_cpu_hlt();
    }
}

#endif /* X86_64_IMPL_KIO_HALT_H */
