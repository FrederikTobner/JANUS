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

#ifndef _STDDEF_H
#define _STDDEF_H

/**
 * @file stddef.h
 * @brief Common definitions (TinyOS freestanding implementation)
 */

/* Size type for sizeof operator */
typedef __SIZE_TYPE__ size_t;

/* Type for pointer arithmetic */
typedef __PTRDIFF_TYPE__ ptrdiff_t;

/* Null pointer constant */
#define NULL ((void*)0)

/* Offset of member in structure */
#define offsetof(type, member) __builtin_offsetof(type, member)

#endif /* _STDDEF_H */
