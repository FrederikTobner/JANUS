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

/// @file clap.h
/// @brief CLAP — CLAP is Likely A Parser
///
/// Declarative command-line argument parsing for C tools.
/// Include this header and declare a static clap_parser_t; the library
/// handles parsing, validation, help generation, and error reporting.

#ifndef CLAP_H
#define CLAP_H

#include <stdbool.h>
#include <stdint.h>

/// The kind of a command-line argument.
typedef enum {
    CLAP_FLAG,       ///< Boolean switch; present or absent. Takes no value.
    CLAP_OPTION,     ///< Named argument that takes exactly one string value.
    CLAP_POSITIONAL, ///< Position-identified argument; no leading "--".
} clap_kind_t;

/// Descriptor for a single argument. Declare as a static constant.
typedef struct {
    clap_kind_t kind;         ///< FLAG, OPTION, or POSITIONAL
    char const * name;        ///< Long name (without "--") and lookup key
    char short_name;          ///< Short single-char form; 0 = no short form
    char const * metavar;     ///< Value placeholder in help, e.g. "FILE"
    char const * help;        ///< One-line description for help output
    char const * default_val; ///< Default value string; NULL = no default
    bool required;            ///< Fatal error if absent after parsing
} clap_arg_t;

/// Top-level parser descriptor. Declare as a static constant.
typedef struct {
    char const * name;       ///< Executable name shown in usage and header
    char const * version;    ///< Version string for --version output
    char const * about;      ///< One-line tool description
    clap_arg_t const * args; ///< Argument descriptor array
    int32_t nargs;           ///< Length of args
} clap_parser_t;

/// Parse result produced by clap_parse(). Free with clap_result_free().
typedef struct {
    char const ** values; ///< Resolved string per arg (parallel to parser->args)
    bool * flags;         ///< Resolved boolean per arg (parallel to parser->args)
} clap_result_t;

/// Boolean flag. No value. Optional by nature.
#define CLAP_FLAG(long_, short_, help_)          {CLAP_FLAG, (long_), (short_), NULL, (help_), NULL, false}

/// Named option. Optional, no default.
#define CLAP_OPTION(long_, short_, meta_, help_) {CLAP_OPTION, (long_), (short_), (meta_), (help_), NULL, false}

/// Named option. Optional, with a default value.
#define CLAP_OPTION_DEFAULT(long_, short_, meta_, help_, def_) \
    {CLAP_OPTION, (long_), (short_), (meta_), (help_), (def_), false}

/// Named option. Required; error if absent.
#define CLAP_REQUIRED(long_, short_, meta_, help_)     {CLAP_OPTION, (long_), (short_), (meta_), (help_), NULL, true}

/// Positional argument. Required.
#define CLAP_POSITIONAL(name_, meta_, help_)           {CLAP_POSITIONAL, (name_), 0, (meta_), (help_), NULL, true}

/// Positional argument. Optional, with a default value.
#define CLAP_POSITIONAL_OPT(name_, meta_, help_, def_) {CLAP_POSITIONAL, (name_), 0, (meta_), (help_), (def_), false}

/// Parse @p argc / @p argv against @p parser.
///
/// Handles --help (prints help, exits 0), --version (prints version, exits 0),
/// and all parse errors (prints message to stderr, exits 1).
/// Returns a heap-allocated result on success; free with clap_result_free().
clap_result_t * clap_parse(clap_parser_t const * parser, int argc, char ** argv);

/// Return the resolved string value for the argument named @p name.
/// Returns NULL if the argument was absent and has no default.
char const * clap_get(clap_result_t const * result, clap_parser_t const * parser, char const * name);

/// Return true if the flag named @p name was present on the command line.
bool clap_flag(clap_result_t const * result, clap_parser_t const * parser, char const * name);

/// Free a result returned by clap_parse(). Safe to call with NULL.
void clap_result_free(clap_result_t * result);

#endif /* CLAP_H */
