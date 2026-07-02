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

/// @file fault_test.c
/// @brief Deliberate-fault self-test implementation (build-flag guarded).

#include <kmain/fault_test.h>

#ifdef JANUS_TEST_FAULTS

#include <janus/types.h>
#include <kio/kio.h>

#ifndef JANUS_FAULT_TEST_KIND
#define JANUS_FAULT_TEST_KIND 0
#endif

#if JANUS_FAULT_TEST_KIND == 1
/// @brief Exhaust the kernel stack via unbounded recursion to provoke a #DF.
///
/// A volatile on-stack buffer that is both written and read prevents the
/// compiler from converting the recursion into a tail call or a loop,
/// guaranteeing that every frame actually consumes stack space.
///
/// @param depth  Recursion counter threaded through each frame; pass 0.
/// @return       Unreachable; present only to defeat tail-call optimisation.
static u64 overflow_stack(u64 depth);
#endif

__noreturn void kmain_fault_test(void)
{
    kprintf("[fault-test] triggering deliberate fault (kind=%d)\n", JANUS_FAULT_TEST_KIND);
#if JANUS_FAULT_TEST_KIND == 1
    // Unbounded recursion exhausts the main kernel stack, escalating to #DF,
    // which must fire on the dedicated IST stack instead of triple-faulting.
    u64 sink = overflow_stack(0);
    kprintf("[fault-test] unreachable %llu\n", (unsigned long long) sink);
#else
    // Write above the first 4 GB → #PF in both boot configurations.
    //
    // Address 0 cannot be used: the multiboot2 boot path identity-maps the
    // first 4 GB with 2 MB huge pages (paging.asm), so a write to virtual
    // address 0 would silently succeed there.  0x100000000 (4 GB + 0) lies
    // just above that window and is unmapped in both the multiboot2
    // (identity-mapped) and limine (higher-half) virtual address spaces.

    u32 volatile * p = (u32 volatile *) 0x100000000ULL;
    *p = 0xDEADBEEFu;
#endif
    for (;;) {
    }
}

#if JANUS_FAULT_TEST_KIND == 1
static u64 overflow_stack(u64 depth)
{
    // A volatile on-stack buffer that is both written and read prevents the
    // compiler from turning this into a tail call / loop, guaranteeing the
    // stack actually grows until it faults.

    u8 volatile guard[256];
    guard[0] = (u8) depth;
    guard[255] = (u8) (depth >> 8);
    u64 next = overflow_stack(depth + 1);
    return next + guard[0] + guard[255];
}
#endif

#endif /* JANUS_TEST_FAULTS */
