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
 * @file file_io_test.cpp
 * @brief Tests for file I/O functionality.
 */

#include <gtest/gtest.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>

extern "C" {
#include "file_io.h"
}

class FileIOTest : public ::testing::Test
{
  protected:
    std::string temp_file;

    void SetUp() override
    {
        // Create a unique temp file name
        temp_file = "/tmp/psf2c_test_XXXXXX";
        int fd = mkstemp(temp_file.data());
        if (fd != -1) {
            close(fd);
        }
    }

    void TearDown() override
    {
        if (!temp_file.empty()) {
            std::remove(temp_file.c_str());
        }
    }

    void WriteFile(uint8_t const * data, size_t size)
    {
        std::ofstream ofs(temp_file, std::ios::binary);
        ofs.write(reinterpret_cast<char const *>(data), size);
    }
};

TEST_F(FileIOTest, LoadRegularFile)
{
    uint8_t const test_data[] = "Hello, World!";
    WriteFile(test_data, sizeof(test_data) - 1); // exclude null terminator

    uint8_t * data = nullptr;
    size_t size = 0;

    ASSERT_EQ(file_load(temp_file.c_str(), &data, &size), 0);
    ASSERT_NE(data, nullptr);
    EXPECT_EQ(size, sizeof(test_data) - 1);
    EXPECT_EQ(memcmp(data, test_data, size), 0);

    free(data);
}

TEST_F(FileIOTest, LoadNonExistentFile)
{
    uint8_t * data = nullptr;
    size_t size = 0;

    EXPECT_EQ(file_load("/nonexistent/path/to/file.psf", &data, &size), -1);
    EXPECT_EQ(data, nullptr);
}

TEST_F(FileIOTest, DetectsGzipMagic)
{
    // Write gzip magic bytes
    uint8_t const gzip_magic[] = {0x1f, 0x8b, 0x08, 0x00};
    WriteFile(gzip_magic, sizeof(gzip_magic));

    EXPECT_NE(file_is_gzip(temp_file.c_str()), 0);
}

TEST_F(FileIOTest, DetectsNonGzip)
{
    uint8_t const not_gzip[] = {0x00, 0x00, 0x00, 0x00};
    WriteFile(not_gzip, sizeof(not_gzip));

    EXPECT_EQ(file_is_gzip(temp_file.c_str()), 0);
}

TEST_F(FileIOTest, GzipDetectionOnNonExistent)
{
    EXPECT_EQ(file_is_gzip("/nonexistent/file"), 0);
}

TEST_F(FileIOTest, LoadEmptyFile)
{
    // Create empty file
    std::ofstream ofs(temp_file, std::ios::binary);
    ofs.close();

    uint8_t * data = nullptr;
    size_t size = 0;

    // Empty file should fail
    EXPECT_EQ(file_load(temp_file.c_str(), &data, &size), -1);
}

// Test loading a valid gzip file (if available on the system)
TEST_F(FileIOTest, LoadGzipFile)
{
    // Check if we have a system gzip file to test with
    char const * test_gz = "/usr/share/kbd/consolefonts/ter-132n.psf.gz";
    if (access(test_gz, R_OK) != 0) {
        GTEST_SKIP() << "Test gzip file not available: " << test_gz;
    }

    uint8_t * data = nullptr;
    size_t size = 0;

    ASSERT_EQ(file_load(test_gz, &data, &size), 0);
    EXPECT_NE(data, nullptr);
    EXPECT_GT(size, 0u);

    // Verify it looks like PSF2 data
    EXPECT_EQ(data[0], 0x72); // PSF2_MAGIC0
    EXPECT_EQ(data[1], 0xb5); // PSF2_MAGIC1

    free(data);
}
