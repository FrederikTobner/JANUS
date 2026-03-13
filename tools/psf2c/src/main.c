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
 * Usage: psf2c <input.psf> <output.h> [symbol_prefix]
 *
 * Converts PSF (PC Screen Font) files to C headers for use in the kernel.
 * Supports PSF1 and PSF2 formats, including gzip-compressed files (.psf.gz).
 */

#include "codegen.h"
#include "file_io.h"
#include "psf.h"

#include <stdio.h>
#include <stdlib.h>

static void print_usage(char const * progname)
{
    fprintf(stderr, "Usage: %s <input.psf> <output.h> [symbol_prefix]\n", progname);
    fprintf(stderr, "\n");
    fprintf(stderr, "Convert a PSF font file to a C header.\n");
    fprintf(stderr, "Supports both raw .psf and gzip-compressed .psf.gz files.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Arguments:\n");
    fprintf(stderr, "  input.psf      Input PSF font file (PSF1 or PSF2, optionally gzipped)\n");
    fprintf(stderr, "  output.h       Output C header file\n");
    fprintf(stderr, "  symbol_prefix  Optional prefix for symbols (default: font)\n");
}

int main(int argc, char * argv[])
{
    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }

    char const * input_path = argv[1];
    char const * output_path = argv[2];
    char const * prefix = (argc >= 4) ? argv[3] : "font";

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

    printf(
        "Font info: %ux%u, %u glyphs, %u bytes/glyph\n", font.width, font.height, font.numglyphs, font.bytesperglyph);

    /* Generate output */
    FILE * fp_out = fopen(output_path, "w");
    if (!fp_out) {
        fprintf(stderr, "Error: Cannot open output file '%s'\n", output_path);
        psf_free(&font);
        return 1;
    }

    codegen_write_header(fp_out, &font, prefix);
    fclose(fp_out);

    printf("Generated: %s\n", output_path);

    psf_free(&font);
    return 0;
}
