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

/// @file kmalloc_test.c
/// @brief Kernel heap allocator self-test implementation (build-flag guarded).

#include <kmain/kmalloc_test.h>

#ifdef JANUS_TEST_KMALLOC

#include <janus/types.h>
#include <kio/die.h>
#include <kio/output.h>
#include <mm/slab_allocator.h>

/// @brief Number of self-contained test cases run by kmain_kmalloc_test().
#define KMALLOC_TEST_CASE_COUNT          11

/// @brief Batch size used by the stats-bookkeeping case (case 10).
#define KMALLOC_TEST_BATCH_SIZE          8

/// @brief Allocation count used by the slab-growth case (case 11).
///
/// KMALLOC_PAGE_USABLE (4048) / 16 bytes = 253 objects per slab; allocating
/// more than that forces the allocator to grow a second backing page.
#define KMALLOC_TEST_SLAB_OVERFLOW_COUNT 300

static void kmalloc_test_case_1_basic_alloc(void);
static void kmalloc_test_case_2_alignment(void);
static void kmalloc_test_case_3_zero_size(void);
static void kmalloc_test_case_4_oversized(void);
static void kmalloc_test_case_5_kcalloc_zero_fill(void);
static void kmalloc_test_case_6_realloc_in_place(void);
static void kmalloc_test_case_7_realloc_grow_class(void);
static void kmalloc_test_case_8_realloc_null_semantics(void);
static void kmalloc_test_case_9_free_null(void);
static void kmalloc_test_case_10_stats_bookkeeping(void);
static void kmalloc_test_case_11_slab_growth_reclaim(void);

void kmain_kmalloc_test(void)
{
    kprintf("[kmalloc-test] running heap allocator smoke tests\n");

    kmalloc_test_case_1_basic_alloc();
    kmalloc_test_case_2_alignment();
    kmalloc_test_case_3_zero_size();
    kmalloc_test_case_4_oversized();
    kmalloc_test_case_5_kcalloc_zero_fill();
    kmalloc_test_case_6_realloc_in_place();
    kmalloc_test_case_7_realloc_grow_class();
    kmalloc_test_case_8_realloc_null_semantics();
    kmalloc_test_case_9_free_null();
    kmalloc_test_case_10_stats_bookkeeping();
    kmalloc_test_case_11_slab_growth_reclaim();

    kprintf("[kmalloc-test] all %d cases passed\n", KMALLOC_TEST_CASE_COUNT);
}

static void kmalloc_test_case_1_basic_alloc(void)
{
    kprintf("[kmalloc-test] case 1: basic alloc/write/free\n");

    void * ptr = kmalloc(64);
    if (ptr == NULL) {
        kpanic("kmalloc-test: case 1 (basic alloc) failed: kmalloc(64) returned NULL");
    }

    u8 * bytes = (u8 *) ptr;
    for (u64 i = 0; i < 64; ++i) {
        bytes[i] = (u8) i;
    }
    for (u64 i = 0; i < 64; ++i) {
        if (bytes[i] != (u8) i) {
            kpanic("kmalloc-test: case 1 (basic alloc) failed: data corruption at offset %lu", i);
        }
    }

    kfree(ptr);
}

static void kmalloc_test_case_2_alignment(void)
{
    kprintf("[kmalloc-test] case 2: alignment across size classes\n");

    static u64 const sizes[] = {16, 32, 64, 128, 256, 512, 1024, 2048, 4048};
    u64 const num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    void * ptrs[sizeof(sizes) / sizeof(sizes[0])];

    for (u64 i = 0; i < num_sizes; ++i) {
        ptrs[i] = kmalloc(sizes[i]);
        if (ptrs[i] == NULL) {
            kpanic("kmalloc-test: case 2 (alignment) failed: kmalloc(%lu) returned NULL", sizes[i]);
        }
        if (((u64) ptrs[i]) & 0xFULL) {
            kpanic("kmalloc-test: case 2 (alignment) failed: pointer for size %lu is misaligned", sizes[i]);
        }
    }

    for (u64 i = 0; i < num_sizes; ++i) {
        kfree(ptrs[i]);
    }
}

static void kmalloc_test_case_3_zero_size(void)
{
    kprintf("[kmalloc-test] case 3: zero-size allocation rejected\n");

    void * ptr = kmalloc(0);
    if (ptr != NULL) {
        kpanic("kmalloc-test: case 3 (zero size) failed: kmalloc(0) returned non-NULL");
    }
}

static void kmalloc_test_case_4_oversized(void)
{
    kprintf("[kmalloc-test] case 4: oversized allocation rejected\n");

    void * ptr = kmalloc(~0ULL);
    if (ptr != NULL) {
        kpanic("kmalloc-test: case 4 (oversized) failed: kmalloc(~0ULL) returned non-NULL");
    }
}

static void kmalloc_test_case_5_kcalloc_zero_fill(void)
{
    kprintf("[kmalloc-test] case 5: kcalloc zero-fills\n");

    void * ptr = kcalloc(128);
    if (ptr == NULL) {
        kpanic("kmalloc-test: case 5 (kcalloc) failed: kcalloc(128) returned NULL");
    }

    u8 * bytes = (u8 *) ptr;
    for (u64 i = 0; i < 128; ++i) {
        if (bytes[i] != 0) {
            kpanic("kmalloc-test: case 5 (kcalloc) failed: non-zero byte at offset %lu", i);
        }
    }

    kfree(ptr);
}

static void kmalloc_test_case_6_realloc_in_place(void)
{
    kprintf("[kmalloc-test] case 6: krealloc grows in place within a size class\n");

    void * ptr = kmalloc(10); // rounds up to the 16-byte class
    if (ptr == NULL) {
        kpanic("kmalloc-test: case 6 (realloc in place) failed: kmalloc(10) returned NULL");
    }
    *(u8 *) ptr = 0xAB;

    void * grown = krealloc(ptr, 16); // still within the 16-byte class -> no move expected
    if (grown != ptr) {
        kpanic("kmalloc-test: case 6 (realloc in place) failed: pointer changed for an in-class grow");
    }
    if (*(u8 *) grown != 0xAB) {
        kpanic("kmalloc-test: case 6 (realloc in place) failed: data not preserved");
    }

    kfree(grown);
}

static void kmalloc_test_case_7_realloc_grow_class(void)
{
    kprintf("[kmalloc-test] case 7: krealloc grows across size classes\n");

    void * ptr = kmalloc(10); // 16-byte class
    if (ptr == NULL) {
        kpanic("kmalloc-test: case 7 (realloc grow class) failed: kmalloc(10) returned NULL");
    }
    *(u8 *) ptr = 0xCD;

    void * grown = krealloc(ptr, 100); // must move to the 128-byte class
    if (grown == NULL) {
        kpanic("kmalloc-test: case 7 (realloc grow class) failed: krealloc(ptr, 100) returned NULL");
    }
    if (*(u8 *) grown != 0xCD) {
        kpanic("kmalloc-test: case 7 (realloc grow class) failed: data not preserved across move");
    }

    kfree(grown);
}

static void kmalloc_test_case_8_realloc_null_semantics(void)
{
    kprintf("[kmalloc-test] case 8: krealloc NULL/zero-size semantics\n");

    void * ptr = krealloc(NULL, 64);
    if (ptr == NULL) {
        kpanic("kmalloc-test: case 8 (realloc semantics) failed: krealloc(NULL, 64) returned NULL");
    }

    void * freed = krealloc(ptr, 0);
    if (freed != NULL) {
        kpanic("kmalloc-test: case 8 (realloc semantics) failed: krealloc(ptr, 0) did not return NULL");
    }
}

static void kmalloc_test_case_9_free_null(void)
{
    kprintf("[kmalloc-test] case 9: kfree(NULL) is a no-op\n");

    kmalloc_stats_t before;
    kmalloc_stats_t after;
    kmalloc_get_stats(&before);

    kfree(NULL);

    kmalloc_get_stats(&after);
    if (before.bytes_in_use != after.bytes_in_use || before.pages_in_use != after.pages_in_use ||
        before.alloc_count != after.alloc_count || before.free_count != after.free_count) {
        kpanic("kmalloc-test: case 9 (free NULL) failed: heap stats changed on kfree(NULL)");
    }
}

static void kmalloc_test_case_10_stats_bookkeeping(void)
{
    kprintf("[kmalloc-test] case 10: stats bookkeeping across a batch\n");

    void * ptrs[KMALLOC_TEST_BATCH_SIZE];

    kmalloc_stats_t before;
    kmalloc_get_stats(&before);

    for (u64 i = 0; i < KMALLOC_TEST_BATCH_SIZE; ++i) {
        ptrs[i] = kmalloc(32);
        if (ptrs[i] == NULL) {
            kpanic("kmalloc-test: case 10 (stats) failed: kmalloc(32) returned NULL at index %lu", i);
        }
    }

    kmalloc_stats_t mid;
    kmalloc_get_stats(&mid);
    if (mid.alloc_count - before.alloc_count != KMALLOC_TEST_BATCH_SIZE) {
        kpanic("kmalloc-test: case 10 (stats) failed: alloc_count delta mismatch");
    }

    for (u64 i = 0; i < KMALLOC_TEST_BATCH_SIZE; ++i) {
        kfree(ptrs[i]);
    }

    kmalloc_stats_t after;
    kmalloc_get_stats(&after);
    if (after.free_count - mid.free_count != KMALLOC_TEST_BATCH_SIZE) {
        kpanic("kmalloc-test: case 10 (stats) failed: free_count delta mismatch");
    }
    if (after.bytes_in_use != before.bytes_in_use) {
        kpanic("kmalloc-test: case 10 (stats) failed: bytes_in_use did not return to baseline");
    }
}

static void kmalloc_test_case_11_slab_growth_reclaim(void)
{
    kprintf("[kmalloc-test] case 11: slab growth and reclaim\n");

    static void * ptrs[KMALLOC_TEST_SLAB_OVERFLOW_COUNT];

    kmalloc_stats_t before;
    kmalloc_get_stats(&before);

    for (u64 i = 0; i < KMALLOC_TEST_SLAB_OVERFLOW_COUNT; ++i) {
        ptrs[i] = kmalloc(16);
        if (ptrs[i] == NULL) {
            kpanic("kmalloc-test: case 11 (slab growth) failed: kmalloc(16) returned NULL at index %lu", i);
        }
    }

    kmalloc_stats_t mid;
    kmalloc_get_stats(&mid);
    if (mid.pages_in_use <= before.pages_in_use) {
        kpanic("kmalloc-test: case 11 (slab growth) failed: pages_in_use did not increase");
    }

    for (u64 i = 0; i < KMALLOC_TEST_SLAB_OVERFLOW_COUNT; ++i) {
        kfree(ptrs[i]);
    }

    kmalloc_stats_t after;
    kmalloc_get_stats(&after);
    if (after.pages_in_use != before.pages_in_use) {
        kpanic("kmalloc-test: case 11 (slab growth) failed: pages_in_use did not return to baseline");
    }
}

#endif /* JANUS_TEST_KMALLOC */
