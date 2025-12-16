/*****************************************************************************
 * Copyright (C) 2025 by Frederik Tobner                                     *
 *                                                                           *
 * This file is part of TinyOS.                                              *
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

#ifndef _STDBOOL_H
#define _STDBOOL_H

/**
 * @file stdbool.h
 * @brief Boolean type (TinyOS freestanding implementation)
 */

#define bool _Bool
#define true 1
#define false 0
#define __bool_true_false_are_defined 1

#endif /* _STDBOOL_H */
