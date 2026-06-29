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

/// @file fault_test.h
/// @brief Deliberate-fault self-test for QEMU verification (build-flag guarded).
///
/// Only declared/compiled when JANUS_TEST_FAULTS is defined. Used to verify the
/// interrupt subsystem turns CPU faults into readable panics instead of resets.

#ifndef KMAIN_FAULT_TEST_H
#define KMAIN_FAULT_TEST_H

#ifdef JANUS_TEST_FAULTS

#include <janus/attributes.h>

/// @brief Trigger a deliberate CPU fault to exercise exception handling.
///
/// The fault kind is selected at build time via JANUS_FAULT_TEST_KIND
/// (0 = page fault via null dereference, 1 = double fault via stack overflow).
/// Never returns: either the fault handler panics, or the function spins.
__noreturn void kmain_fault_test(void);

#endif /* JANUS_TEST_FAULTS */

#endif /* KMAIN_FAULT_TEST_H */
