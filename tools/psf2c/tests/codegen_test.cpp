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
 * @file codegen_test.cpp
 * @brief Tests for C header code generation.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

extern "C" {
#include "codegen.h"
#include "psf.h"
}

class CodegenTest : public ::testing::Test
{
  protected:
    psf_font_t font{};
    std::string temp_file;
    FILE * output_file = nullptr;

    void SetUp() override
    {
        // Create a temp file
        temp_file = "/tmp/psf2c_codegen_test_XXXXXX";
        int fd = mkstemp(temp_file.data());
        if (fd != -1) {
            close(fd);
        }
    }

    void TearDown() override
    {
        if (output_file) {
            fclose(output_file);
            output_file = nullptr;
        }
        if (!temp_file.empty()) {
            std::remove(temp_file.c_str());
        }
        if (font.glyphs) {
            free(font.glyphs);
        }
    }

    std::string ReadOutput()
    {
        std::ifstream ifs(temp_file);
        std::stringstream buffer;
        buffer << ifs.rdbuf();
        return buffer.str();
    }

    /// @brief Open the temp file, generate a header for @p font with @p prefix,
    /// close the file, and return the full output as a string.
    std::string GenerateOutput(char const * prefix)
    {
        output_file = fopen(temp_file.c_str(), "w");
        EXPECT_NE(output_file, nullptr) << "fopen failed for: " << temp_file;
        if (output_file == nullptr) {
            return {};
        }
        codegen_write_header(output_file, &font, prefix);
        fclose(output_file);
        output_file = nullptr;
        return ReadOutput();
    }

    void SetupSimpleFont()
    {
        font.width = 8;
        font.height = 16;
        font.numglyphs = 2;
        font.bytesperglyph = 16;
        font.glyphs = static_cast<uint8_t *>(calloc(font.numglyphs, font.bytesperglyph));
    }
};

TEST_F(CodegenTest, GeneratesHeaderGuard)
{
    // Arrange
    SetupSimpleFont();

    // Act
    std::string output = GenerateOutput("testfont");

    // Assert
    EXPECT_THAT(output, ::testing::HasSubstr("#ifndef TESTFONT_H"));
    EXPECT_THAT(output, ::testing::HasSubstr("#define TESTFONT_H"));
    EXPECT_THAT(output, ::testing::HasSubstr("#endif /* TESTFONT_H */"));
}

TEST_F(CodegenTest, GeneratesMacros)
{
    // Arrange
    SetupSimpleFont();

    // Act
    std::string output = GenerateOutput("myfont");

    // Assert
    EXPECT_THAT(output, ::testing::HasSubstr("#define MYFONT_WIDTH  8"));
    EXPECT_THAT(output, ::testing::HasSubstr("#define MYFONT_HEIGHT 16"));
    EXPECT_THAT(output, ::testing::HasSubstr("#define MYFONT_GLYPH_COUNT 2"));
    EXPECT_THAT(output, ::testing::HasSubstr("#define MYFONT_BYTES_PER_GLYPH 16"));
}

TEST_F(CodegenTest, GeneratesArrayDeclaration)
{
    // Arrange
    SetupSimpleFont();

    // Act
    std::string output = GenerateOutput("font");

    // Assert
    EXPECT_THAT(output, ::testing::HasSubstr("static const u8 font_glyphs[2][16]"));
}

TEST_F(CodegenTest, IncludesJanusTypes)
{
    // Arrange
    SetupSimpleFont();

    // Act
    std::string output = GenerateOutput("font");

    // Assert
    EXPECT_THAT(output, ::testing::HasSubstr("#include <janus/types.h>"));
}

TEST_F(CodegenTest, PrintableAsciiComments)
{
    // Arrange
    font.width = 8;
    font.height = 8;
    font.numglyphs = 128;
    font.bytesperglyph = 8;
    font.glyphs = static_cast<uint8_t *>(calloc(font.numglyphs, font.bytesperglyph));

    // Act
    std::string output = GenerateOutput("font");

    // Assert
    EXPECT_THAT(output, ::testing::HasSubstr("/* 0x20 ' ' */"));    // space
    EXPECT_THAT(output, ::testing::HasSubstr("/* 0x41 'A' */"));    // A
    EXPECT_THAT(output, ::testing::HasSubstr("/* 0x5C '\\\\' */")); // backslash escaped
    EXPECT_THAT(output, ::testing::HasSubstr("/* 0x27 '\\'"));      // single quote escaped
}

TEST_F(CodegenTest, NonPrintableNoCharComment)
{
    // Arrange
    SetupSimpleFont();

    // Act
    std::string output = GenerateOutput("font");

    // Assert — glyph 0x00 must not include a character in its comment
    EXPECT_THAT(output, ::testing::HasSubstr("/* 0x00 */ {"));
}

TEST_F(CodegenTest, UppercasePrefixConversion)
{
    // Arrange
    SetupSimpleFont();

    // Act
    std::string output = GenerateOutput("myLowerCase");

    // Assert
    EXPECT_THAT(output, ::testing::HasSubstr("MYLOWERCASE_WIDTH"));  // macros use uppercase
    EXPECT_THAT(output, ::testing::HasSubstr("myLowerCase_glyphs")); // array uses original case
}
