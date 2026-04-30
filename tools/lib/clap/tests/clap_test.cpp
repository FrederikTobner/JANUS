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
// Shared parser fixture
// ---------------------------------------------------------------------------

static clap_arg_t const TEST_ARGS[] = {
    CLAP_POSITIONAL("input", "FILE", "Input file"),
    CLAP_POSITIONAL("output", "FILE", "Output file"),
    CLAP_POSITIONAL_OPT("prefix", "STRING", "Symbol prefix", "font"),
    CLAP_FLAG("verbose", 'v', "Enable verbose output"),
    CLAP_OPTION_DEFAULT("format", 'f', "FMT", "Output format", "c"),
};

static clap_parser_t const TEST_PARSER = {
    .name    = "test",
    .version = "1.0.0",
    .about   = "Test parser",
    .args    = TEST_ARGS,
    .nargs   = (int32_t)(sizeof TEST_ARGS / sizeof *TEST_ARGS),
};

/// Build a mutable argv vector from a list of string literals.
/// The returned vector is null-terminated and owns no strings.
static std::vector<char *> make_argv(std::initializer_list<char const *> args)
{
    std::vector<char *> argv;
    for (char const * s : args) {
        argv.push_back(const_cast<char *>(s));
    }
    argv.push_back(nullptr);
    return argv;
}

// ---------------------------------------------------------------------------
// Happy-path tests
// ---------------------------------------------------------------------------

TEST(ClapParse, RequiredPositionals)
{
    auto argv   = make_argv({"test", "in.psf", "out.h"});
    int32_t argc = (int32_t)(argv.size() - 1);
    clap_result_t * result = clap_parse(&TEST_PARSER, argc, argv.data());
    ASSERT_NE(result, nullptr);
    EXPECT_STREQ(clap_get(result, &TEST_PARSER, "input"),  "in.psf");
    EXPECT_STREQ(clap_get(result, &TEST_PARSER, "output"), "out.h");
    clap_result_free(result);
}

TEST(ClapParse, DefaultPositionalUsedWhenAbsent)
{
    auto argv    = make_argv({"test", "in.psf", "out.h"});
    int32_t argc = (int32_t)(argv.size() - 1);
    clap_result_t * result = clap_parse(&TEST_PARSER, argc, argv.data());
    ASSERT_NE(result, nullptr);
    EXPECT_STREQ(clap_get(result, &TEST_PARSER, "prefix"), "font");
    clap_result_free(result);
}

TEST(ClapParse, OptionalPositionalOverridesDefault)
{
    auto argv    = make_argv({"test", "in.psf", "out.h", "myfont"});
    int32_t argc = (int32_t)(argv.size() - 1);
    clap_result_t * result = clap_parse(&TEST_PARSER, argc, argv.data());
    ASSERT_NE(result, nullptr);
    EXPECT_STREQ(clap_get(result, &TEST_PARSER, "prefix"), "myfont");
    clap_result_free(result);
}

TEST(ClapParse, FlagLongForm)
{
    auto argv    = make_argv({"test", "in.psf", "out.h", "--verbose"});
    int32_t argc = (int32_t)(argv.size() - 1);
    clap_result_t * result = clap_parse(&TEST_PARSER, argc, argv.data());
    ASSERT_NE(result, nullptr);
    EXPECT_TRUE(clap_flag(result, &TEST_PARSER, "verbose"));
    clap_result_free(result);
}

TEST(ClapParse, FlagShortForm)
{
    auto argv    = make_argv({"test", "in.psf", "out.h", "-v"});
    int32_t argc = (int32_t)(argv.size() - 1);
    clap_result_t * result = clap_parse(&TEST_PARSER, argc, argv.data());
    ASSERT_NE(result, nullptr);
    EXPECT_TRUE(clap_flag(result, &TEST_PARSER, "verbose"));
    clap_result_free(result);
}

TEST(ClapParse, FlagAbsentByDefault)
{
    auto argv    = make_argv({"test", "in.psf", "out.h"});
    int32_t argc = (int32_t)(argv.size() - 1);
    clap_result_t * result = clap_parse(&TEST_PARSER, argc, argv.data());
    ASSERT_NE(result, nullptr);
    EXPECT_FALSE(clap_flag(result, &TEST_PARSER, "verbose"));
    clap_result_free(result);
}

TEST(ClapParse, OptionLongFormEquals)
{
    auto argv    = make_argv({"test", "in.psf", "out.h", "--format=bin"});
    int32_t argc = (int32_t)(argv.size() - 1);
    clap_result_t * result = clap_parse(&TEST_PARSER, argc, argv.data());
    ASSERT_NE(result, nullptr);
    EXPECT_STREQ(clap_get(result, &TEST_PARSER, "format"), "bin");
    clap_result_free(result);
}

TEST(ClapParse, OptionLongFormSpace)
{
    auto argv    = make_argv({"test", "in.psf", "out.h", "--format", "bin"});
    int32_t argc = (int32_t)(argv.size() - 1);
    clap_result_t * result = clap_parse(&TEST_PARSER, argc, argv.data());
    ASSERT_NE(result, nullptr);
    EXPECT_STREQ(clap_get(result, &TEST_PARSER, "format"), "bin");
    clap_result_free(result);
}

TEST(ClapParse, OptionDefaultUsedWhenAbsent)
{
    auto argv    = make_argv({"test", "in.psf", "out.h"});
    int32_t argc = (int32_t)(argv.size() - 1);
    clap_result_t * result = clap_parse(&TEST_PARSER, argc, argv.data());
    ASSERT_NE(result, nullptr);
    EXPECT_STREQ(clap_get(result, &TEST_PARSER, "format"), "c");
    clap_result_free(result);
}

TEST(ClapParse, EndOfOptionsMarker)
{
    // Tokens after "--" are treated as positionals even if they look like flags.
    auto argv    = make_argv({"test", "in.psf", "--", "out.h"});
    int32_t argc = (int32_t)(argv.size() - 1);
    clap_result_t * result = clap_parse(&TEST_PARSER, argc, argv.data());
    ASSERT_NE(result, nullptr);
    EXPECT_STREQ(clap_get(result, &TEST_PARSER, "input"),  "in.psf");
    EXPECT_STREQ(clap_get(result, &TEST_PARSER, "output"), "out.h");
    clap_result_free(result);
}

TEST(ClapParse, OptionsAndPositionalsMixed)
{
    auto argv    = make_argv({"test", "in.psf", "--verbose", "out.h", "--format=bin"});
    int32_t argc = (int32_t)(argv.size() - 1);
    clap_result_t * result = clap_parse(&TEST_PARSER, argc, argv.data());
    ASSERT_NE(result, nullptr);
    EXPECT_STREQ(clap_get(result, &TEST_PARSER, "input"),  "in.psf");
    EXPECT_STREQ(clap_get(result, &TEST_PARSER, "output"), "out.h");
    EXPECT_TRUE(clap_flag(result, &TEST_PARSER, "verbose"));
    EXPECT_STREQ(clap_get(result, &TEST_PARSER, "format"), "bin");
    clap_result_free(result);
}

// ---------------------------------------------------------------------------
// Error-path tests (death tests — clap exits on bad input)
// ---------------------------------------------------------------------------

TEST(ClapParseDeath, UnknownLongOption)
{
    auto argv    = make_argv({"test", "in.psf", "out.h", "--unknown"});
    int32_t argc = (int32_t)(argv.size() - 1);
    EXPECT_EXIT(clap_parse(&TEST_PARSER, argc, argv.data()),
                ::testing::ExitedWithCode(1), "");
}

TEST(ClapParseDeath, UnknownShortOption)
{
    auto argv    = make_argv({"test", "in.psf", "out.h", "-z"});
    int32_t argc = (int32_t)(argv.size() - 1);
    EXPECT_EXIT(clap_parse(&TEST_PARSER, argc, argv.data()),
                ::testing::ExitedWithCode(1), "");
}

TEST(ClapParseDeath, MissingRequiredPositional)
{
    // Only one positional provided; "output" is required.
    auto argv    = make_argv({"test", "in.psf"});
    int32_t argc = (int32_t)(argv.size() - 1);
    EXPECT_EXIT(clap_parse(&TEST_PARSER, argc, argv.data()),
                ::testing::ExitedWithCode(1), "");
}

TEST(ClapParseDeath, TooManyPositionals)
{
    auto argv    = make_argv({"test", "in.psf", "out.h", "prefix", "extra"});
    int32_t argc = (int32_t)(argv.size() - 1);
    EXPECT_EXIT(clap_parse(&TEST_PARSER, argc, argv.data()),
                ::testing::ExitedWithCode(1), "");
}

TEST(ClapParseDeath, ShortOptionBundledCharsRejected)
{
    // "-vv" must be rejected; only single-character short options are accepted.
    auto argv    = make_argv({"test", "in.psf", "out.h", "-vv"});
    int32_t argc = (int32_t)(argv.size() - 1);
    EXPECT_EXIT(clap_parse(&TEST_PARSER, argc, argv.data()),
                ::testing::ExitedWithCode(1), "");
}

TEST(ClapParseDeath, ShortOptionCombinedCharsRejected)
{
    // "-vf" must be rejected; multi-character short tokens are not supported.
    auto argv    = make_argv({"test", "in.psf", "out.h", "-vf"});
    int32_t argc = (int32_t)(argv.size() - 1);
    EXPECT_EXIT(clap_parse(&TEST_PARSER, argc, argv.data()),
                ::testing::ExitedWithCode(1), "");
}

TEST(ClapParseDeath, OptionMissingValue)
{
    // "--format" without a following value must be rejected.
    auto argv    = make_argv({"test", "in.psf", "out.h", "--format"});
    int32_t argc = (int32_t)(argv.size() - 1);
    EXPECT_EXIT(clap_parse(&TEST_PARSER, argc, argv.data()),
                ::testing::ExitedWithCode(1), "");
}

TEST(ClapParseDeath, FlagDoesNotAcceptEqualsValue)
{
    // "--verbose=yes" must be rejected; flags take no value.
    auto argv    = make_argv({"test", "in.psf", "out.h", "--verbose=yes"});
    int32_t argc = (int32_t)(argv.size() - 1);
    EXPECT_EXIT(clap_parse(&TEST_PARSER, argc, argv.data()),
                ::testing::ExitedWithCode(1), "");
}
