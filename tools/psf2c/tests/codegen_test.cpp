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
 * @file codegen_test.cpp
 * @brief Tests for C header code generation.
 */

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
    SetupSimpleFont();

    output_file = fopen(temp_file.c_str(), "w");
    ASSERT_NE(output_file, nullptr);

    codegen_write_header(output_file, &font, "testfont");
    fclose(output_file);
    output_file = nullptr;

    std::string output = ReadOutput();

    EXPECT_NE(output.find("#ifndef TESTFONT_H"), std::string::npos);
    EXPECT_NE(output.find("#define TESTFONT_H"), std::string::npos);
    EXPECT_NE(output.find("#endif /* TESTFONT_H */"), std::string::npos);
}

TEST_F(CodegenTest, GeneratesMacros)
{
    SetupSimpleFont();

    output_file = fopen(temp_file.c_str(), "w");
    ASSERT_NE(output_file, nullptr);

    codegen_write_header(output_file, &font, "myfont");
    fclose(output_file);
    output_file = nullptr;

    std::string output = ReadOutput();

    EXPECT_NE(output.find("#define MYFONT_WIDTH  8"), std::string::npos);
    EXPECT_NE(output.find("#define MYFONT_HEIGHT 16"), std::string::npos);
    EXPECT_NE(output.find("#define MYFONT_GLYPH_COUNT 2"), std::string::npos);
    EXPECT_NE(output.find("#define MYFONT_BYTES_PER_GLYPH 16"), std::string::npos);
}

TEST_F(CodegenTest, GeneratesArrayDeclaration)
{
    SetupSimpleFont();

    output_file = fopen(temp_file.c_str(), "w");
    ASSERT_NE(output_file, nullptr);

    codegen_write_header(output_file, &font, "font");
    fclose(output_file);
    output_file = nullptr;

    std::string output = ReadOutput();

    EXPECT_NE(output.find("static const u8 font_glyphs[2][16]"), std::string::npos);
}

TEST_F(CodegenTest, IncludesJanusTypes)
{
    SetupSimpleFont();

    output_file = fopen(temp_file.c_str(), "w");
    ASSERT_NE(output_file, nullptr);

    codegen_write_header(output_file, &font, "font");
    fclose(output_file);
    output_file = nullptr;

    std::string output = ReadOutput();

    EXPECT_NE(output.find("#include <janus/types.h>"), std::string::npos);
}

TEST_F(CodegenTest, PrintableAsciiComments)
{
    font.width = 8;
    font.height = 8;
    font.numglyphs = 128;
    font.bytesperglyph = 8;
    font.glyphs = static_cast<uint8_t *>(calloc(font.numglyphs, font.bytesperglyph));

    output_file = fopen(temp_file.c_str(), "w");
    ASSERT_NE(output_file, nullptr);

    codegen_write_header(output_file, &font, "font");
    fclose(output_file);
    output_file = nullptr;

    std::string output = ReadOutput();

    // Check that printable ASCII chars have comments
    EXPECT_NE(output.find("/* 0x20 ' ' */"), std::string::npos);    // space
    EXPECT_NE(output.find("/* 0x41 'A' */"), std::string::npos);    // A
    EXPECT_NE(output.find("/* 0x5C '\\\\' */"), std::string::npos); // backslash escaped
    EXPECT_NE(output.find("/* 0x27 '\\'"), std::string::npos);      // single quote escaped
}

TEST_F(CodegenTest, NonPrintableNoCharComment)
{
    SetupSimpleFont();

    output_file = fopen(temp_file.c_str(), "w");
    ASSERT_NE(output_file, nullptr);

    codegen_write_header(output_file, &font, "font");
    fclose(output_file);
    output_file = nullptr;

    std::string output = ReadOutput();

    // Glyph 0x00 should not have a character in the comment
    EXPECT_NE(output.find("/* 0x00 */ {"), std::string::npos);
}

TEST_F(CodegenTest, UppercasePrefixConversion)
{
    SetupSimpleFont();

    output_file = fopen(temp_file.c_str(), "w");
    ASSERT_NE(output_file, nullptr);

    codegen_write_header(output_file, &font, "myLowerCase");
    fclose(output_file);
    output_file = nullptr;

    std::string output = ReadOutput();

    // Macros should use uppercase
    EXPECT_NE(output.find("MYLOWERCASE_WIDTH"), std::string::npos);
    // Array should use original case
    EXPECT_NE(output.find("myLowerCase_glyphs"), std::string::npos);
}
