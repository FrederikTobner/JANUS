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

#include <kio/output.h>
#include <ktest/ktest.h>

extern ktest_case_t const __start_ktests[];
extern ktest_case_t const __stop_ktests[];

void ktest__fail(ktest_ctx_t * ctx, char const * file, int line, char const * expr)
{
    ctx->failed = true;
    kprintf("    %s:%d: %s\n", file, line, expr);
}

__noreturn void ktest_run_all(void)
{
    u64 total = 0;
    u64 passed = 0;
    u64 failed = 0;

    kprintf("\n==== ktest ====\n");
    for (ktest_case_t const * current = __start_ktests; current < __stop_ktests; ++current) {
        ktest_ctx_t ctx = {.failed = false};
        kprintf("[ RUN  ] %s.%s\n", current->suite, current->name);
        current->fn(&ctx);
        ++total;

        if (ctx.failed) {
            ++failed;
            kprintf("[ FAIL ] %s.%s\n", current->suite, current->name);
        } else {
            ++passed;
            kprintf("[  OK  ] %s.%s\n", current->suite, current->name);
        }
    }

    kprintf("\nKTEST SUMMARY: %llu run, %llu passed, %llu failed\n",
            (unsigned long long) total,
            (unsigned long long) passed,
            (unsigned long long) failed);

    if (failed == 0) {
        kprintf("KTEST: PASS\n");
        ktest_exit(true);
    }

    kprintf("KTEST: FAIL %llu\n", (unsigned long long) failed);
    ktest_exit(false);
    __builtin_unreachable();
}
