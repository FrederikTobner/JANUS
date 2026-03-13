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

#ifndef BOOT_VERIFY_H
#define BOOT_VERIFY_H

#include <janus/types.h>

/**
 * Verify that the boot handoff is valid for the currently supported protocols.
 *
 * Returns 0 on success, non-zero on failure.
 */
error_t boot_verify_handoff(u64 loader_magic, void * info);

#endif /* BOOT_VERIFY_H */
