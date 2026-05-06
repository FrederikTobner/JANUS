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

/// @file arch/impl/asm/capabilities.h
/// @brief AArch64 asm capability flags.

#ifndef AARCH64_IMPL_ASM_CAPABILITIES_H
#define AARCH64_IMPL_ASM_CAPABILITIES_H

#define ASM_ARCH_X86_64             0
#define ASM_ARCH_AARCH64            1

#define ASM_CAP_LOCAL_IRQ_CONTROL   1
#define ASM_CAP_IDLE_WAIT_INTERRUPT 1
#define ASM_CAP_TLB_INVALIDATE_PAGE 1
#define ASM_CAP_PORT_IO             0
#define ASM_CAP_ARCH_SYSREG_ACCESS  1

#endif /* AARCH64_IMPL_ASM_CAPABILITIES_H */
