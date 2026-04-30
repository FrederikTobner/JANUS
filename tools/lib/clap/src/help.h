/*****************************************************************************
 * Copyright (C) 2026 by Frederik Tobner                                     *
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

/// @file help.h
/// @brief Internal declarations for help output.
/// Not part of the public interface; do not install or include externally.

#ifndef CLAP_HELP_H
#define CLAP_HELP_H

#include <clap/clap.h>

/// Fixed left-column width for help output (characters).
#define CLAP_HELP_COL 24

/// @brief Print help for @p parser to stdout, then exit(0).
/// @param parser Parser descriptor.
void clap_print_help(clap_parser_t const * parser);

/// @brief Print version for @p parser to stdout, then exit(0).
/// @param parser Parser descriptor.
void clap_print_version(clap_parser_t const * parser);

#endif /* CLAP_HELP_H */
