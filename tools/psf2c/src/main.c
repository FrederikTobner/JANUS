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
 * @file main.c
 * @brief PSF to C header converter - entry point.
 *
 * Usage: psf2c <input.psf> <output.h> [prefix] [--verbose]
 *
 * Converts PSF (PC Screen Font) files to C headers for use in the kernel.
 * Supports PSF1 and PSF2 formats, including gzip-compressed files (.psf.gz).
 */

#include "codegen.h"
#include "file_io.h"
#include "psf.h"

#include <clap/clap.h>
#include <stdio.h>
#include <stdlib.h>

static clap_arg_t const ARGS[] = {
    CLAP_POSITIONAL("input", "FILE", "Input PSF font file (.psf or .psf.gz)"),
    CLAP_POSITIONAL("output", "FILE", "Output C header file"),
    CLAP_POSITIONAL_OPT("prefix", "STRING", "Symbol prefix for generated identifiers", "font"),
    CLAP_FLAG("verbose", 'v', "Print font information after conversion"),
};

static clap_parser_t const PARSER = {
    .name = "psf2c",
    .version = "0.1.0",
    .about = "Convert a PSF font file to a C header for use in the kernel",
    .args = ARGS,
    .nargs = (int32_t) (sizeof ARGS / sizeof *ARGS),
};

int main(int argc, char * argv[])
{
    clap_result_t * result = clap_parse(&PARSER, argc, argv);

    char const * input_path = clap_get(result, &PARSER, "input");
    char const * output_path = clap_get(result, &PARSER, "output");
    char const * prefix = clap_get(result, &PARSER, "prefix");
    bool verbose = clap_flag(result, &PARSER, "verbose");

    // Safe to free immediately: pointers remain valid (into argv or static defaults).
    clap_result_free(result);

    /* Load file (with automatic gzip detection) */
    uint8_t * file_data = NULL;
    size_t file_size = 0;
    if (file_load(input_path, &file_data, &file_size) != 0) {
        return 1;
    }

    /* Parse PSF data */
    psf_font_t font;
    if (psf_parse(file_data, file_size, &font) != 0) {
        free(file_data);
        return 1;
    }
    free(file_data);

    if (verbose) {
        printf("Font info: %ux%u, %u glyphs, %u bytes/glyph\n",
               font.width,
               font.height,
               font.numglyphs,
               font.bytesperglyph);
    }

    /* Generate output */
    FILE * fp_out = fopen(output_path, "w");
    if (!fp_out) {
        fprintf(stderr, "error: cannot open output file '%s'\n", output_path);
        psf_free(&font);
        return 1;
    }

    codegen_write_header(fp_out, &font, prefix);
    fclose(fp_out);

    printf("Generated: %s\n", output_path);

    psf_free(&font);
    return 0;
}
