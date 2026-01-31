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
 * @file arch/drivers/cpu.h
 * @brief CPU control architecture contract.
 *
 * This file acts as a bridge - it just includes the architecture-specific
 * implementation header. The include path determines which architecture
 * is used (set by build system).
 *
 * Include chain: <drivers/cpu.h> → <arch/drivers/cpu.h> → <arch/impl/drivers/cpu.h>
 */

#ifndef ARCH_DRIVERS_CPU_H
#define ARCH_DRIVERS_CPU_H

// Pull in the architecture-specific implementation
// Resolved via include path: -I drivers/arch/x86_64/include
#include <arch/impl/drivers/cpu.h>

#endif /* ARCH_DRIVERS_CPU_H */
