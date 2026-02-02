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
 * @file codegen.h
 * @brief C header code generation for font data.
 */

#ifndef CODEGEN_H
#define CODEGEN_H

#include "psf.h"

#include <stdio.h>

/**
 * @brief Generate a C header file containing font data.
 *
 * @param out     Output file stream
 * @param font    Parsed font data
 * @param prefix  Symbol prefix (e.g., "terminus" generates TERMINUS_WIDTH, terminus_glyphs, etc.)
 */
void codegen_write_header(FILE * out, psf_font_t const * font, char const * prefix);

#endif /* CODEGEN_H */
