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

#include <asm/capabilities.h>
#include <asm/cpu.h>
#include <asm/io.h>
#include <janus/attributes.h>
#include <janus/types.h>
#include <ktest/ktest.h>

#define KTEST_QEMU_EXIT_PORT 0xF4
#define KTEST_QEMU_PASS_VAL  0x10
#define KTEST_QEMU_FAIL_VAL  0x11

__noreturn void ktest_exit(bool passed)
{
    // Under AArch64, this value is unused, but we still need to mark it as used to avoid warnings.
    (void) passed;
#if ASM_CAP_PORT_IO
    asm_io_outb(KTEST_QEMU_EXIT_PORT, (u8) (passed ? KTEST_QEMU_PASS_VAL : KTEST_QEMU_FAIL_VAL));
#endif

#if ASM_CAP_IDLE_WAIT_INTERRUPT && ASM_CAP_LOCAL_IRQ_CONTROL
    asm_cpu_halt_forever();
#else
    for (;;) {
    }
#endif
}
