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
 * @file psf.h
 * @brief PSF (PC Screen Font) format parsing.
 */

#ifndef PSF_H
#define PSF_H

#include <stddef.h>
#include <stdint.h>

/* PSF1 constants */
#define PSF1_MAGIC0     0x36
#define PSF1_MAGIC1     0x04
#define PSF1_MODE512    0x01
#define PSF1_MODEHASTAB 0x02

/* PSF2 constants */
#define PSF2_MAGIC0     0x72
#define PSF2_MAGIC1     0xb5
#define PSF2_MAGIC2     0x4a
#define PSF2_MAGIC3     0x86

/**
 * @brief Parsed font information.
 */
typedef struct {
    uint32_t width;         /**< Glyph width in pixels */
    uint32_t height;        /**< Glyph height in pixels */
    uint32_t numglyphs;     /**< Number of glyphs */
    uint32_t bytesperglyph; /**< Bytes per glyph bitmap */
    uint8_t * glyphs;       /**< Glyph bitmap data (caller must free) */
} psf_font_t;

/**
 * @brief Parse PSF font data from a memory buffer.
 *
 * Supports both PSF1 and PSF2 formats. On success, the caller is responsible
 * for freeing font->glyphs.
 *
 * @param data  Raw font file data
 * @param size  Size of data in bytes
 * @param font  Output font information
 * @return 0 on success, -1 on error
 */
int psf_parse(uint8_t const * data, size_t size, psf_font_t * font);

/**
 * @brief Free resources associated with a parsed font.
 *
 * @param font Font to free
 */
void psf_free(psf_font_t * font);

#endif /* PSF_H */
