/*****************************************************************************
 * Copyright (C) 2025 by Frederik Tobner                                     *
 *                                                                           *
 * This file is part of JANUS                                                *
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

#ifndef JANUS_ERRNO_H
#define JANUS_ERRNO_H

/// @file errno.h
/// @brief Kernel error code constants.
///
/// Named error codes for the @c error_t type defined in @c types.h.
/// Convention: negative values are errors, zero is success, positive values
/// carry informational status (reserved for future use).
///
/// Values are chosen to match the negated POSIX errno convention used by the
/// Linux kernel for in-kernel error returns, simplifying future comparisons
/// with upstream kernel code.

/// @brief Success — no error.
#define JANUS_OK      0

/// @brief Invalid argument passed to a function.
#define JANUS_EINVAL  (-1)

/// @brief Out of memory or internal resource pool exhausted.
#define JANUS_ENOMEM  (-2)

/// @brief Hardware I/O error or device operation failed.
#define JANUS_EIO     (-3)

/// @brief Device not present, failed self-test, or not responding.
#define JANUS_ENODEV  (-4)

/// @brief Resource temporarily unavailable or already in use.
#define JANUS_EBUSY   (-5)

/// @brief Invalid address or alignment error.
#define JANUS_EFAULT  (-6)

/// @brief Operation not supported or not implemented.
#define JANUS_ENOSYS  (-7)

/// @brief Resource or address range exhausted.
#define JANUS_ENOSPC  (-8)

#endif /* JANUS_ERRNO_H */
