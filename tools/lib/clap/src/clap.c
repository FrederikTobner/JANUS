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

/// @file clap.c
/// @brief Core parsing, validation, accessors, memory management, error reporting.

#include "help.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/// @brief Return the index of the argument with the given long @p name, or -1.
/// @param parser Parser descriptor.
/// @param name Long option name (without "--").
/// @return Index in parser->args, or -1 if not found.
static int32_t find_by_long(clap_parser_t const * parser, char const * name);

/// @brief Return the index of the argument with the given short form character, or -1.
/// @param parser Parser descriptor.
/// @param ch Short option character.
/// @return Index in parser->args, or -1 if not found.
static int32_t find_by_short(clap_parser_t const * parser, char ch);

/// @brief Return the index of the n-th CLAP_POSITIONAL in declaration order, or -1.
/// @param parser Parser descriptor.
/// @param n Zero-based positional index.
/// @return Index in parser->args, or -1 if not found.
static int32_t find_nth_positional(clap_parser_t const * parser, int32_t n);

/// @brief Print a formatted error to stderr and exit(1).
/// @param parser Parser descriptor (used for the usage hint line).
/// @param fmt printf-style format string for the error detail.
static void clap_die(clap_parser_t const * parser, char const * fmt, ...);

clap_result_t * clap_parse(clap_parser_t const * parser, int argc, char ** argv)
{
    clap_result_t * result = malloc(sizeof(clap_result_t));
    if (!result) {
        abort();
    }

    result->values = calloc((size_t) parser->nargs, sizeof(char const *));
    result->flags = calloc((size_t) parser->nargs, sizeof(bool));
    if (!result->values || !result->flags) {
        abort();
    }

    // Apply defaults before parsing so absent optional args are already set.
    for (int32_t i = 0; i < parser->nargs; i++) {
        if (parser->args[i].default_val) {
            result->values[i] = parser->args[i].default_val;
        }
    }

    bool end_of_options = false;
    int32_t positional_cursor = 0;

    for (int32_t i = 1; i < argc; i++) {
        char const * token = argv[i];

        if (!end_of_options && strcmp(token, "--") == 0) {
            end_of_options = true;
            continue;
        }

        if (!end_of_options && strncmp(token, "--", 2) == 0) {
            char const * raw = token + 2;
            char const * eq = strchr(raw, '=');

            if (eq) {
                size_t namelen = (size_t) (eq - raw);
                char name[64];
                if (namelen >= sizeof name) {
                    clap_die(parser, "option name too long");
                }
                memcpy(name, raw, namelen);
                name[namelen] = '\0';

                int32_t idx = find_by_long(parser, name);
                if (idx < 0) {
                    clap_die(parser, "unknown option: --%s", name);
                }
                if (parser->args[idx].kind == CLAP_FLAG) {
                    clap_die(parser, "flag --%s does not accept a value", name);
                }
                result->values[idx] = eq + 1;

            } else {
                if (strcmp(raw, "help") == 0) {
                    clap_print_help(parser);
                }
                if (strcmp(raw, "version") == 0) {
                    clap_print_version(parser);
                }

                int32_t idx = find_by_long(parser, raw);
                if (idx < 0) {
                    clap_die(parser, "unknown option: --%s", raw);
                }
                if (parser->args[idx].kind == CLAP_FLAG) {
                    result->flags[idx] = true;
                } else {
                    i++;
                    if (i >= argc) {
                        clap_die(parser, "option --%s requires a value", raw);
                    }
                    result->values[idx] = argv[i];
                }
            }
            continue;
        }

        if (!end_of_options && token[0] == '-' && token[1] != '\0') {
            if (token[2] != '\0') {
                clap_die(parser, "invalid option: '%s' (short options must be a single character)", token);
            }
            char ch = token[1];
            if (ch == 'h') {
                clap_print_help(parser);
            }

            int32_t idx = find_by_short(parser, ch);
            if (idx < 0) {
                clap_die(parser, "unknown option: -%c", ch);
            }
            if (parser->args[idx].kind == CLAP_FLAG) {
                result->flags[idx] = true;
            } else {
                i++;
                if (i >= argc) {
                    clap_die(parser, "option -%c requires a value", ch);
                }
                result->values[idx] = argv[i];
            }
            continue;
        }

        int32_t pos_idx = find_nth_positional(parser, positional_cursor);
        if (pos_idx < 0) {
            clap_die(parser, "unexpected argument: %s", token);
        }
        result->values[pos_idx] = token;
        positional_cursor++;
    }

    // Validate: all required arguments must be resolved.
    for (int32_t i = 0; i < parser->nargs; i++) {
        clap_arg_t const * arg = &parser->args[i];
        if (arg->required && !result->values[i]) {
            if (arg->kind == CLAP_POSITIONAL) {
                clap_die(parser, "missing required argument <%s>", arg->name);
            } else {
                clap_die(parser, "missing required option --%s", arg->name);
            }
        }
    }

    return result;
}

char const * clap_get(clap_result_t const * result, clap_parser_t const * parser, char const * name)
{
    for (int32_t i = 0; i < parser->nargs; i++) {
        if (strcmp(parser->args[i].name, name) == 0) {
            return result->values[i];
        }
    }
    return NULL;
}

bool clap_flag(clap_result_t const * result, clap_parser_t const * parser, char const * name)
{
    for (int32_t i = 0; i < parser->nargs; i++) {
        if (strcmp(parser->args[i].name, name) == 0) {
            return result->flags[i];
        }
    }
    return false;
}

void clap_result_free(clap_result_t * result)
{
    if (!result) {
        return;
    }
    // values and flags point into argv or static default strings; not owned.
    free(result->values);
    free(result->flags);
    free(result);
}

static int32_t find_by_long(clap_parser_t const * parser, char const * name)
{
    for (int32_t i = 0; i < parser->nargs; i++) {
        if (strcmp(parser->args[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

static int32_t find_by_short(clap_parser_t const * parser, char ch)
{
    for (int32_t i = 0; i < parser->nargs; i++) {
        if (parser->args[i].short_name == ch) {
            return i;
        }
    }
    return -1;
}

static int32_t find_nth_positional(clap_parser_t const * parser, int32_t n)
{
    int32_t count = 0;
    for (int32_t i = 0; i < parser->nargs; i++) {
        if (parser->args[i].kind == CLAP_POSITIONAL) {
            if (count == n) {
                return i;
            }
            count++;
        }
    }
    return -1;
}

static void clap_die(clap_parser_t const * parser, char const * fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "error: ");
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\nTry '%s --help' for more information.\n", parser->name);
    exit(1);
}
