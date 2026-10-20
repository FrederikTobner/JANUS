/*****************************************************************************
 * Copyright (C) 2026 by Frederik Tobner                                     *
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

#include <ktest/ktest.h>

// Each helper runs one assertion on a private context; its failure line on the serial log is expected noise.

static void assert_true_body(ktest_ctx_t * _kt, bool * reached, int value)
{
    KTEST_ASSERT_TRUE(value);
    *reached = true;
}

static void assert_eq_body(ktest_ctx_t * _kt, bool * reached, int lhs, int rhs)
{
    KTEST_ASSERT_EQ(lhs, rhs);
    *reached = true;
}

static void assert_ne_body(ktest_ctx_t * _kt, bool * reached, int lhs, int rhs)
{
    KTEST_ASSERT_NE(lhs, rhs);
    *reached = true;
}

KTEST(ktest, passing_assert_keeps_context_clean)
{
    ktest_ctx_t ctx = {.failed = false};
    bool reached = false;
    assert_true_body(&ctx, &reached, 1);
    KTEST_ASSERT_FALSE(ctx.failed);
    KTEST_ASSERT_TRUE(reached);
}

KTEST(ktest, failing_assert_true_marks_context_and_returns)
{
    ktest_ctx_t ctx = {.failed = false};
    bool reached = false;
    assert_true_body(&ctx, &reached, 0);
    KTEST_ASSERT_TRUE(ctx.failed);
    KTEST_ASSERT_FALSE(reached);
}

KTEST(ktest, failing_assert_eq_marks_context_and_returns)
{
    ktest_ctx_t ctx = {.failed = false};
    bool reached = false;
    assert_eq_body(&ctx, &reached, 1, 2);
    KTEST_ASSERT_TRUE(ctx.failed);
    KTEST_ASSERT_FALSE(reached);
}

KTEST(ktest, failing_assert_ne_marks_context_and_returns)
{
    ktest_ctx_t ctx = {.failed = false};
    bool reached = false;
    assert_ne_body(&ctx, &reached, 3, 3);
    KTEST_ASSERT_TRUE(ctx.failed);
    KTEST_ASSERT_FALSE(reached);
}
