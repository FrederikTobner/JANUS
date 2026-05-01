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
 * @file clap_test.cpp
 * @brief Unit tests for the CLAP command-line argument parser.
 */

#include <gtest/gtest.h>

#include <initializer_list>
#include <vector>

extern "C" {
#include <clap/clap.h>
}

// ---------------------------------------------------------------------------
// Shared fixture
// ---------------------------------------------------------------------------

/// @brief Fixture shared by all parse tests.
///
/// Provides a fully populated parser with two required positionals, one
/// optional positional (default "font"), one flag, and one option with a
/// default value.  Each test builds an argv via MakeArgv() and is responsible
/// for freeing the result with clap_result_free().
class ClapParseTest : public ::testing::Test
{
  protected:
    static clap_arg_t const kArgs[];
    static clap_parser_t const kParser;

    /// @brief Build a mutable argv vector from a list of string literals.
    /// The returned vector is null-terminated; no strings are owned.
    static std::vector<char *> MakeArgv(std::initializer_list<char const *> args)
    {
        std::vector<char *> argv;
        for (char const * s : args) {
            argv.push_back(const_cast<char *>(s));
        }
        argv.push_back(nullptr);
        return argv;
    }
};

clap_arg_t const ClapParseTest::kArgs[] = {
    CLAP_POSITIONAL("input", "FILE", "Input file"),
    CLAP_POSITIONAL("output", "FILE", "Output file"),
    CLAP_POSITIONAL_OPT("prefix", "STRING", "Symbol prefix", "font"),
    CLAP_FLAG("verbose", 'v', "Enable verbose output"),
    CLAP_OPTION_DEFAULT("format", 'f', "FMT", "Output format", "c"),
};

clap_parser_t const ClapParseTest::kParser = {
    .name = "test",
    .version = "1.0.0",
    .about = "Test parser",
    .args = ClapParseTest::kArgs,
    .nargs = (int32_t) (sizeof ClapParseTest::kArgs / sizeof *ClapParseTest::kArgs),
};

// ---------------------------------------------------------------------------
// Happy-path tests
// ---------------------------------------------------------------------------

TEST_F(ClapParseTest, ParsesRequiredPositionals)
{
    // Arrange
    auto argv = MakeArgv({"test", "in.psf", "out.h"});
    int32_t argc = (int32_t) (argv.size() - 1);

    // Act
    clap_result_t * result = clap_parse(&kParser, argc, argv.data());

    // Assert
    ASSERT_NE(result, nullptr);
    EXPECT_STREQ(clap_get(result, &kParser, "input"), "in.psf");
    EXPECT_STREQ(clap_get(result, &kParser, "output"), "out.h");
    clap_result_free(result);
}

TEST_F(ClapParseTest, UsesDefaultForAbsentOptionalPositional)
{
    // Arrange
    auto argv = MakeArgv({"test", "in.psf", "out.h"});
    int32_t argc = (int32_t) (argv.size() - 1);

    // Act
    clap_result_t * result = clap_parse(&kParser, argc, argv.data());

    // Assert
    ASSERT_NE(result, nullptr);
    EXPECT_STREQ(clap_get(result, &kParser, "prefix"), "font");
    clap_result_free(result);
}

TEST_F(ClapParseTest, OverridesDefaultWhenOptionalPositionalProvided)
{
    // Arrange
    auto argv = MakeArgv({"test", "in.psf", "out.h", "myfont"});
    int32_t argc = (int32_t) (argv.size() - 1);

    // Act
    clap_result_t * result = clap_parse(&kParser, argc, argv.data());

    // Assert
    ASSERT_NE(result, nullptr);
    EXPECT_STREQ(clap_get(result, &kParser, "prefix"), "myfont");
    clap_result_free(result);
}

TEST_F(ClapParseTest, SetsFlagFromLongForm)
{
    // Arrange
    auto argv = MakeArgv({"test", "in.psf", "out.h", "--verbose"});
    int32_t argc = (int32_t) (argv.size() - 1);

    // Act
    clap_result_t * result = clap_parse(&kParser, argc, argv.data());

    // Assert
    ASSERT_NE(result, nullptr);
    EXPECT_TRUE(clap_flag(result, &kParser, "verbose"));
    clap_result_free(result);
}

TEST_F(ClapParseTest, SetsFlagFromShortForm)
{
    // Arrange
    auto argv = MakeArgv({"test", "in.psf", "out.h", "-v"});
    int32_t argc = (int32_t) (argv.size() - 1);

    // Act
    clap_result_t * result = clap_parse(&kParser, argc, argv.data());

    // Assert
    ASSERT_NE(result, nullptr);
    EXPECT_TRUE(clap_flag(result, &kParser, "verbose"));
    clap_result_free(result);
}

TEST_F(ClapParseTest, FlagIsFalseWhenAbsent)
{
    // Arrange
    auto argv = MakeArgv({"test", "in.psf", "out.h"});
    int32_t argc = (int32_t) (argv.size() - 1);

    // Act
    clap_result_t * result = clap_parse(&kParser, argc, argv.data());

    // Assert
    ASSERT_NE(result, nullptr);
    EXPECT_FALSE(clap_flag(result, &kParser, "verbose"));
    clap_result_free(result);
}

TEST_F(ClapParseTest, ParsesOptionLongFormWithEquals)
{
    // Arrange
    auto argv = MakeArgv({"test", "in.psf", "out.h", "--format=bin"});
    int32_t argc = (int32_t) (argv.size() - 1);

    // Act
    clap_result_t * result = clap_parse(&kParser, argc, argv.data());

    // Assert
    ASSERT_NE(result, nullptr);
    EXPECT_STREQ(clap_get(result, &kParser, "format"), "bin");
    clap_result_free(result);
}

TEST_F(ClapParseTest, ParsesOptionLongFormWithSpace)
{
    // Arrange
    auto argv = MakeArgv({"test", "in.psf", "out.h", "--format", "bin"});
    int32_t argc = (int32_t) (argv.size() - 1);

    // Act
    clap_result_t * result = clap_parse(&kParser, argc, argv.data());

    // Assert
    ASSERT_NE(result, nullptr);
    EXPECT_STREQ(clap_get(result, &kParser, "format"), "bin");
    clap_result_free(result);
}

TEST_F(ClapParseTest, UsesDefaultForAbsentOption)
{
    // Arrange
    auto argv = MakeArgv({"test", "in.psf", "out.h"});
    int32_t argc = (int32_t) (argv.size() - 1);

    // Act
    clap_result_t * result = clap_parse(&kParser, argc, argv.data());

    // Assert
    ASSERT_NE(result, nullptr);
    EXPECT_STREQ(clap_get(result, &kParser, "format"), "c");
    clap_result_free(result);
}

TEST_F(ClapParseTest, TreatsTokensAfterDashDashAsPositionals)
{
    // Arrange — "out.h" follows "--" and looks like it could be a flag start,
    // but must be consumed as the second positional.
    auto argv = MakeArgv({"test", "in.psf", "--", "out.h"});
    int32_t argc = (int32_t) (argv.size() - 1);

    // Act
    clap_result_t * result = clap_parse(&kParser, argc, argv.data());

    // Assert
    ASSERT_NE(result, nullptr);
    EXPECT_STREQ(clap_get(result, &kParser, "input"), "in.psf");
    EXPECT_STREQ(clap_get(result, &kParser, "output"), "out.h");
    clap_result_free(result);
}

TEST_F(ClapParseTest, ParsesInterleavedPositionalsAndOptions)
{
    // Arrange
    auto argv = MakeArgv({"test", "in.psf", "--verbose", "out.h", "--format=bin"});
    int32_t argc = (int32_t) (argv.size() - 1);

    // Act
    clap_result_t * result = clap_parse(&kParser, argc, argv.data());

    // Assert
    ASSERT_NE(result, nullptr);
    EXPECT_STREQ(clap_get(result, &kParser, "input"), "in.psf");
    EXPECT_STREQ(clap_get(result, &kParser, "output"), "out.h");
    EXPECT_TRUE(clap_flag(result, &kParser, "verbose"));
    EXPECT_STREQ(clap_get(result, &kParser, "format"), "bin");
    clap_result_free(result);
}

// ---------------------------------------------------------------------------
// Error-path tests (death tests — clap exits(1) on bad input)
// ---------------------------------------------------------------------------

/// @brief Fixture for death tests; inherits the same parser and MakeArgv helper.
using ClapParseDeathTest = ClapParseTest;

TEST_F(ClapParseDeathTest, RejectsUnknownLongOption)
{
    // Arrange
    auto argv = MakeArgv({"test", "in.psf", "out.h", "--unknown"});
    int32_t argc = (int32_t) (argv.size() - 1);

    // Act + Assert
    EXPECT_EXIT(clap_parse(&kParser, argc, argv.data()), ::testing::ExitedWithCode(1), "unknown option");
}

TEST_F(ClapParseDeathTest, RejectsUnknownShortOption)
{
    // Arrange
    auto argv = MakeArgv({"test", "in.psf", "out.h", "-z"});
    int32_t argc = (int32_t) (argv.size() - 1);

    // Act + Assert
    EXPECT_EXIT(clap_parse(&kParser, argc, argv.data()), ::testing::ExitedWithCode(1), "unknown option");
}

TEST_F(ClapParseDeathTest, RejectsMissingRequiredPositional)
{
    // Arrange — only "input" provided; "output" is required.
    auto argv = MakeArgv({"test", "in.psf"});
    int32_t argc = (int32_t) (argv.size() - 1);

    // Act + Assert
    EXPECT_EXIT(clap_parse(&kParser, argc, argv.data()), ::testing::ExitedWithCode(1), "missing required");
}

TEST_F(ClapParseDeathTest, RejectsTooManyPositionals)
{
    // Arrange — four positionals provided; only three are declared.
    auto argv = MakeArgv({"test", "in.psf", "out.h", "prefix", "extra"});
    int32_t argc = (int32_t) (argv.size() - 1);

    // Act + Assert
    EXPECT_EXIT(clap_parse(&kParser, argc, argv.data()), ::testing::ExitedWithCode(1), "unexpected argument");
}

TEST_F(ClapParseDeathTest, RejectsBundledShortOptions)
{
    // Arrange — "-vv" is not supported; only single-character short options.
    auto argv = MakeArgv({"test", "in.psf", "out.h", "-vv"});
    int32_t argc = (int32_t) (argv.size() - 1);

    // Act + Assert
    EXPECT_EXIT(clap_parse(&kParser, argc, argv.data()), ::testing::ExitedWithCode(1), "invalid option");
}

TEST_F(ClapParseDeathTest, RejectsCombinedShortOptions)
{
    // Arrange — "-vf" combines two separate flags; not supported.
    auto argv = MakeArgv({"test", "in.psf", "out.h", "-vf"});
    int32_t argc = (int32_t) (argv.size() - 1);

    // Act + Assert
    EXPECT_EXIT(clap_parse(&kParser, argc, argv.data()), ::testing::ExitedWithCode(1), "invalid option");
}

TEST_F(ClapParseDeathTest, RejectsOptionWithMissingValue)
{
    // Arrange — "--format" with no following token.
    auto argv = MakeArgv({"test", "in.psf", "out.h", "--format"});
    int32_t argc = (int32_t) (argv.size() - 1);

    // Act + Assert
    EXPECT_EXIT(clap_parse(&kParser, argc, argv.data()), ::testing::ExitedWithCode(1), "requires a value");
}

TEST_F(ClapParseDeathTest, RejectsFlagWithEqualsValue)
{
    // Arrange — "--verbose=yes" must be rejected; flags accept no value.
    auto argv = MakeArgv({"test", "in.psf", "out.h", "--verbose=yes"});
    int32_t argc = (int32_t) (argv.size() - 1);

    // Act + Assert
    EXPECT_EXIT(clap_parse(&kParser, argc, argv.data()), ::testing::ExitedWithCode(1), "does not accept a value");
}
