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

/**
 * @file psf_test.cpp
 * @brief Tests for PSF parsing functionality.
 */

#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

extern "C" {
#include "psf.h"
}

class PsfParseTest : public ::testing::Test
{
  protected:
    psf_font_t font{};

    void TearDown() override
    {
        psf_free(&font);
    }
};

// ---------------------------------------------------------------------------
// Happy-path tests
// ---------------------------------------------------------------------------

TEST_F(PsfParseTest, RejectsTruncatedPsf1Data)
{
    // Arrange — PSF1 mode=0 declares 256 glyphs (charsize=8 → 2 048 bytes of
    // glyph data), but only two glyphs are supplied.
    static uint8_t const kPsf1Truncated[] = {
        PSF1_MAGIC0,
        PSF1_MAGIC1,
        0x00, // mode (256 glyphs, no unicode table)
        0x08, // charsize (8 bytes per glyph)
        // Glyph 0
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        // Glyph 1
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
    };

    // Act
    int result = psf_parse(kPsf1Truncated, sizeof(kPsf1Truncated), &font);

    // Assert
    EXPECT_EQ(result, -1);
}

/// @brief Minimal well-formed PSF2 font: 1 glyph, 8×16.
static uint8_t const kValidPsf2[] = {
    // Magic
    PSF2_MAGIC0,
    PSF2_MAGIC1,
    PSF2_MAGIC2,
    PSF2_MAGIC3,
    // Version
    0x00,
    0x00,
    0x00,
    0x00,
    // Header size = 32
    0x20,
    0x00,
    0x00,
    0x00,
    // Flags
    0x00,
    0x00,
    0x00,
    0x00,
    // Number of glyphs = 1
    0x01,
    0x00,
    0x00,
    0x00,
    // Bytes per glyph = 16
    0x10,
    0x00,
    0x00,
    0x00,
    // Height = 16
    0x10,
    0x00,
    0x00,
    0x00,
    // Width = 8
    0x08,
    0x00,
    0x00,
    0x00,
    // Glyph data (16 bytes)
    0x00,
    0x18,
    0x3C,
    0x66,
    0x66,
    0x7E,
    0x66,
    0x66,
    0x66,
    0x66,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
};

TEST_F(PsfParseTest, ParsesValidPsf2)
{
    // Act
    int result = psf_parse(kValidPsf2, sizeof(kValidPsf2), &font);

    // Assert
    ASSERT_EQ(result, 0);
    EXPECT_EQ(font.width, 8u);
    EXPECT_EQ(font.height, 16u);
    EXPECT_EQ(font.numglyphs, 1u);
    EXPECT_EQ(font.bytesperglyph, 16u);
    EXPECT_NE(font.glyphs, nullptr);
}

TEST_F(PsfParseTest, RejectsNullData)
{
    // Act
    int result = psf_parse(nullptr, 100, &font);

    // Assert
    EXPECT_EQ(result, -1);
}

// ---------------------------------------------------------------------------
// Rejection tests (parameterized)
// ---------------------------------------------------------------------------

/// @brief Describes a single invalid PSF byte buffer.
struct InvalidPsfInput {
    char const * name;
    std::vector<uint8_t> bytes;
};

/// @brief Parameterized fixture for inputs that must be rejected.
class PsfInvalidInputTest : public ::testing::TestWithParam<InvalidPsfInput>
{
  protected:
    psf_font_t font{};

    void TearDown() override
    {
        psf_free(&font);
    }
};

TEST_P(PsfInvalidInputTest, ReturnsErrorCode)
{
    // Arrange
    InvalidPsfInput const & param = GetParam();

    // Act
    int result = psf_parse(param.bytes.data(), param.bytes.size(), &font);

    // Assert
    EXPECT_EQ(result, -1);
}

static InvalidPsfInput const kInvalidInputs[] = {
    {
        "InvalidMagic",
        {0xDE, 0xAD, 0xBE, 0xEF},
    },
    {
        "TooSmall",
        {PSF2_MAGIC0, PSF2_MAGIC1},
    },
    {
        "TruncatedPsf2Header",
        {PSF2_MAGIC0, PSF2_MAGIC1, PSF2_MAGIC2, PSF2_MAGIC3, 0x00, 0x00, 0x00, 0x00},
    },
    {
        "TruncatedGlyphData",
        {// Magic
         PSF2_MAGIC0,
         PSF2_MAGIC1,
         PSF2_MAGIC2,
         PSF2_MAGIC3,
         // Version
         0x00,
         0x00,
         0x00,
         0x00,
         // Header size = 32
         0x20,
         0x00,
         0x00,
         0x00,
         // Flags
         0x00,
         0x00,
         0x00,
         0x00,
         // Number of glyphs = 1
         0x01,
         0x00,
         0x00,
         0x00,
         // Bytes per glyph = 16
         0x10,
         0x00,
         0x00,
         0x00,
         // Height = 16
         0x10,
         0x00,
         0x00,
         0x00,
         // Width = 8
         0x08,
         0x00,
         0x00,
         0x00},
        // (no glyph data follows)
    },
};

INSTANTIATE_TEST_SUITE_P(InvalidPsfInputs,
                         PsfInvalidInputTest,
                         ::testing::ValuesIn(kInvalidInputs),
                         [](::testing::TestParamInfo<InvalidPsfInput> const & info) { return info.param.name; });

// ---------------------------------------------------------------------------
// psf_free edge cases
// ---------------------------------------------------------------------------

class PsfFreeTest : public ::testing::Test
{
  protected:
    psf_font_t font{};
};

TEST_F(PsfFreeTest, HandlesNullPointer)
{
    // Act + Assert — must not crash
    psf_free(nullptr);
}

TEST_F(PsfFreeTest, HandlesNullGlyphs)
{
    // Arrange
    font.glyphs = nullptr;

    // Act + Assert — must not crash
    psf_free(&font);
}
