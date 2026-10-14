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

#ifndef KTEST_KTEST_H
#define KTEST_KTEST_H

#include <janus/attributes.h>
#include <janus/types.h>

/// @brief Per-test execution state.
typedef struct ktest_ctx {
    bool failed;
} ktest_ctx_t;

/// @brief One registered kernel test.
typedef struct ktest_case {
    char const * suite;
    char const * name;
    void (*fn)(ktest_ctx_t * ctx);
} ktest_case_t;

void ktest__fail(ktest_ctx_t * ctx, char const * file, int line, char const * expr);

#define KTEST(suite_id, name_id)                                                               \
    static void ktest_fn_##suite_id##_##name_id(ktest_ctx_t * _kt);                            \
    static ktest_case_t const ktest_case_##suite_id##_##name_id __used __section("ktests") = { \
        .suite = #suite_id,                                                                    \
        .name = #name_id,                                                                      \
        .fn = ktest_fn_##suite_id##_##name_id,                                                 \
    };                                                                                         \
    static void ktest_fn_##suite_id##_##name_id(ktest_ctx_t * _kt)

#define KTEST_ASSERT_TRUE(cond)                                             \
    do {                                                                    \
        if (!(cond)) {                                                      \
            ktest__fail(_kt, __FILE__, __LINE__, "ASSERT_TRUE(" #cond ")"); \
            return;                                                         \
        }                                                                   \
    } while (0)

#define KTEST_ASSERT_FALSE(cond) KTEST_ASSERT_TRUE(!(cond))

#define KTEST_ASSERT_EQ(a, b)                                                  \
    do {                                                                       \
        if ((a) != (b)) {                                                      \
            ktest__fail(_kt, __FILE__, __LINE__, "ASSERT_EQ(" #a ", " #b ")"); \
            return;                                                            \
        }                                                                      \
    } while (0)

#define KTEST_ASSERT_NE(a, b)                                                  \
    do {                                                                       \
        if ((a) == (b)) {                                                      \
            ktest__fail(_kt, __FILE__, __LINE__, "ASSERT_NE(" #a ", " #b ")"); \
            return;                                                            \
        }                                                                      \
    } while (0)

__noreturn void ktest_run_all(void);
void ktest_exit(bool passed);

#endif /* KTEST_KTEST_H */
