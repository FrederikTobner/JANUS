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

/// @file help.c
/// @brief Help and version output rendering.

#include "help.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/// @brief Print @p left padded to CLAP_HELP_COL characters, then @p right.
/// @param left Left-column text (option name and flags).
/// @param right Description text.
static void print_row(char const * left, char const * right);

/// @brief Build a left-column entry for a flag or option into @p buf.
/// @param arg Argument descriptor.
/// @param buf Output buffer (size at least CLAP_HELP_COL + 16).
/// @param bufsz Size of @p buf.
/// @return @p buf.
static char * build_option_left(clap_arg_t const * arg, char * buf, size_t bufsz);

/// @brief Print @p help with an optional "[default: @p default_val]" suffix.
/// @param help Description string.
/// @param default_val Default value string, or NULL for none.
static void print_description(char const * help, char const * default_val);

void clap_print_help(clap_parser_t const * parser)
{
    printf("%s %s\n", parser->name, parser->version);
    printf("%s\n\n", parser->about);

    printf("Usage: %s", parser->name);
    for (int32_t i = 0; i < parser->nargs; i++) {
        clap_arg_t const * a = &parser->args[i];
        if (a->kind != CLAP_POSITIONAL) {
            continue;
        }
        if (a->required) {
            printf(" <%s>", a->name);
        } else {
            printf(" [%s]", a->name);
        }
    }
    for (int32_t i = 0; i < parser->nargs; i++) {
        if (parser->args[i].kind == CLAP_FLAG || parser->args[i].kind == CLAP_OPTION) {
            printf(" [OPTIONS]");
            break;
        }
    }
    printf("\n");

    bool has_positionals = false;
    for (int32_t i = 0; i < parser->nargs; i++) {
        if (parser->args[i].kind == CLAP_POSITIONAL) {
            has_positionals = true;
            break;
        }
    }
    if (has_positionals) {
        printf("\nArguments:\n");
        for (int32_t i = 0; i < parser->nargs; i++) {
            clap_arg_t const * a = &parser->args[i];
            if (a->kind != CLAP_POSITIONAL) {
                continue;
            }
            char left[64];
            if (a->required) {
                snprintf(left, sizeof left, "  <%s>", a->metavar ? a->metavar : a->name);
            } else {
                snprintf(left, sizeof left, "  [%s]", a->metavar ? a->metavar : a->name);
            }
            fputs(left, stdout);
            int32_t pad = CLAP_HELP_COL - (int32_t) strlen(left);
            for (int32_t p = 0; p < pad; p++) {
                putchar(' ');
            }
            if (pad <= 0) {
                putchar(' ');
            }
            print_description(a->help, a->default_val);
            putchar('\n');
        }
    }

    printf("\nOptions:\n");
    char left[64];
    for (int32_t i = 0; i < parser->nargs; i++) {
        clap_arg_t const * a = &parser->args[i];
        if (a->kind != CLAP_FLAG && a->kind != CLAP_OPTION) {
            continue;
        }
        build_option_left(a, left, sizeof left);
        fputs(left, stdout);
        int32_t pad = CLAP_HELP_COL - (int32_t) strlen(left);
        for (int32_t p = 0; p < pad; p++) {
            putchar(' ');
        }
        if (pad <= 0) {
            putchar(' ');
        }
        print_description(a->help, a->default_val);
        putchar('\n');
    }
    print_row("      --help", "Print help");
    print_row("      --version", "Print version information");

    exit(0);
}

void clap_print_version(clap_parser_t const * parser)
{
    printf("%s %s\n", parser->name, parser->version);
    exit(0);
}

static void print_row(char const * left, char const * right)
{
    int32_t len = (int32_t) strlen(left);
    fputs(left, stdout);
    if (len < CLAP_HELP_COL) {
        for (int32_t i = len; i < CLAP_HELP_COL; i++) {
            putchar(' ');
        }
    } else {
        putchar(' ');
    }
    puts(right);
}

static char * build_option_left(clap_arg_t const * arg, char * buf, size_t bufsz)
{
    if (arg->short_name) {
        snprintf(buf, bufsz, "  -%c, --%s", arg->short_name, arg->name);
    } else {
        snprintf(buf, bufsz, "      --%s", arg->name);
    }
    if (arg->kind == CLAP_OPTION && arg->metavar) {
        size_t used = strlen(buf);
        snprintf(buf + used, bufsz - used, " <%s>", arg->metavar);
    }
    return buf;
}

static void print_description(char const * help, char const * default_val)
{
    if (default_val) {
        char buf[256];
        snprintf(buf, sizeof buf, "%s [default: %s]", help, default_val);
        fputs(buf, stdout);
    } else {
        fputs(help, stdout);
    }
}
