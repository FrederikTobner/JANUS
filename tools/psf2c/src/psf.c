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
 * @file psf.c
 * @brief PSF (PC Screen Font) format parsing implementation.
 */

#include "psf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* PSF1 header */
struct psf1_header {
    uint8_t magic[2];
    uint8_t mode;
    uint8_t charsize;
};

/* PSF2 header */
struct psf2_header {
    uint8_t magic[4];
    uint32_t version;
    uint32_t headersize;
    uint32_t flags;
    uint32_t numglyph;
    uint32_t bytesperglyph;
    uint32_t height;
    uint32_t width;
};

static int psf_parse_v2(uint8_t const * data, size_t size, psf_font_t * font)
{
    if (size < sizeof(struct psf2_header)) {
        fprintf(stderr, "Error: File too small for PSF2 header\n");
        return -1;
    }

    struct psf2_header const * hdr = (struct psf2_header const *) data;
    printf("Detected PSF2 format\n");

    font->width = hdr->width;
    font->height = hdr->height;
    font->numglyphs = hdr->numglyph;
    font->bytesperglyph = hdr->bytesperglyph;

    size_t data_size = hdr->numglyph * hdr->bytesperglyph;
    if (hdr->headersize + data_size > size) {
        fprintf(stderr, "Error: File too small for glyph data\n");
        return -1;
    }

    font->glyphs = malloc(data_size);
    if (!font->glyphs) {
        fprintf(stderr, "Error: Out of memory\n");
        return -1;
    }
    memcpy(font->glyphs, data + hdr->headersize, data_size);
    return 0;
}

static int psf_parse_v1(uint8_t const * data, size_t size, psf_font_t * font)
{
    if (size < sizeof(struct psf1_header)) {
        fprintf(stderr, "Error: File too small for PSF1 header\n");
        return -1;
    }

    struct psf1_header const * hdr = (struct psf1_header const *) data;
    printf("Detected PSF1 format\n");

    size_t glyph_count = (hdr->mode & PSF1_MODE512) ? 512 : 256;

    font->width = 8;
    font->height = hdr->charsize;
    font->numglyphs = glyph_count;
    font->bytesperglyph = hdr->charsize;

    size_t data_size = glyph_count * hdr->charsize;
    if (sizeof(struct psf1_header) + data_size > size) {
        fprintf(stderr, "Error: File too small for glyph data\n");
        return -1;
    }

    font->glyphs = malloc(data_size);
    if (!font->glyphs) {
        fprintf(stderr, "Error: Out of memory\n");
        return -1;
    }
    memcpy(font->glyphs, data + sizeof(struct psf1_header), data_size);
    return 0;
}

int psf_parse(uint8_t const * data, size_t size, psf_font_t * font)
{
    if (!data || size < 4 || !font) {
        fprintf(stderr, "Error: Invalid arguments\n");
        return -1;
    }

    /* Initialize output */
    memset(font, 0, sizeof(*font));

    /* Check for PSF2 */
    if (data[0] == PSF2_MAGIC0 && data[1] == PSF2_MAGIC1 && data[2] == PSF2_MAGIC2 && data[3] == PSF2_MAGIC3) {
        return psf_parse_v2(data, size, font);
    }

    /* Check for PSF1 */
    if (data[0] == PSF1_MAGIC0 && data[1] == PSF1_MAGIC1) {
        return psf_parse_v1(data, size, font);
    }

    fprintf(stderr,
            "Error: Unknown file format (magic: %02x %02x %02x %02x)\n",
            data[0],
            data[1],
            size > 2 ? data[2] : 0,
            size > 3 ? data[3] : 0);
    return -1;
}

void psf_free(psf_font_t * font)
{
    if (font && font->glyphs) {
        free(font->glyphs);
        font->glyphs = NULL;
    }
}
