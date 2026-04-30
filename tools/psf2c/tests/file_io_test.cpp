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
 * @file file_io_test.cpp
 * @brief Tests for file I/O functionality.
 */

#include <gtest/gtest.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <unistd.h>

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
    // Arrange
    static uint8_t const kTestData[] = "Hello, World!";
    WriteFile(kTestData, sizeof(kTestData) - 1); // exclude null terminator

    uint8_t * data = nullptr;
    size_t    size = 0;

    // Act
    ASSERT_EQ(file_load(temp_file.c_str(), &data, &size), 0);

    // Assert
    ASSERT_NE(data, nullptr);
    EXPECT_EQ(size, sizeof(kTestData) - 1);
    EXPECT_EQ(memcmp(data, kTestData, size), 0);

    free(data);
}

TEST_F(FileIOTest, LoadNonExistentFile)
{
    // Arrange
    uint8_t * data = nullptr;
    size_t    size = 0;

    // Act
    int result = file_load("/nonexistent/path/to/file.psf", &data, &size);

    // Assert
    EXPECT_EQ(result, -1);
    EXPECT_EQ(data, nullptr);
}

TEST_F(FileIOTest, DetectsGzipMagic)
{
    // Arrange
    static uint8_t const kGzipMagic[] = {0x1f, 0x8b, 0x08, 0x00};
    WriteFile(kGzipMagic, sizeof(kGzipMagic));

    // Act + Assert
    EXPECT_NE(file_is_gzip(temp_file.c_str()), 0);
}

TEST_F(FileIOTest, DetectsNonGzip)
{
    // Arrange
    static uint8_t const kNotGzip[] = {0x00, 0x00, 0x00, 0x00};
    WriteFile(kNotGzip, sizeof(kNotGzip));

    // Act + Assert
    EXPECT_EQ(file_is_gzip(temp_file.c_str()), 0);
}

TEST_F(FileIOTest, GzipDetectionOnNonExistent)
{
    // Act + Assert
    EXPECT_EQ(file_is_gzip("/nonexistent/file"), 0);
}

TEST_F(FileIOTest, LoadEmptyFile)
{
    // Arrange
    std::ofstream ofs(temp_file, std::ios::binary);
    ofs.close();

    uint8_t * data = nullptr;
    size_t    size = 0;

    // Act
    int result = file_load(temp_file.c_str(), &data, &size);

    // Assert
    EXPECT_EQ(result, -1);
}

TEST_F(FileIOTest, LoadGzipFile)
{
    // Arrange
    static constexpr char const * kTestGzPath = "/usr/share/kbd/consolefonts/ter-132n.psf.gz";
    if (access(kTestGzPath, R_OK) != 0) {
        GTEST_SKIP() << "Test gzip file not available: " << kTestGzPath;
    }

    uint8_t * data = nullptr;
    size_t    size = 0;

    // Act
    ASSERT_EQ(file_load(kTestGzPath, &data, &size), 0);

    // Assert
    ASSERT_NE(data, nullptr);
    EXPECT_GT(size, 0u);
    EXPECT_EQ(data[0], 0x72u); // PSF2_MAGIC0
    EXPECT_EQ(data[1], 0xb5u); // PSF2_MAGIC1

    free(data);
}
