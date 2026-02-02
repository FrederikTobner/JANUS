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
 * @file file_io.c
 * @brief File loading utilities with gzip support.
 */

#include "file_io.h"

#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>

/* Gzip magic bytes */
#define GZIP_MAGIC0 0x1f
#define GZIP_MAGIC1 0x8b

int file_is_gzip(char const * path)
{
    FILE * fp = fopen(path, "rb");
    if (!fp) {
        return 0;
    }

    uint8_t magic[2];
    int is_gz = (fread(magic, 1, 2, fp) == 2 && magic[0] == GZIP_MAGIC0 && magic[1] == GZIP_MAGIC1);
    fclose(fp);
    return is_gz;
}

static int load_gzip(char const * path, uint8_t ** out_data, size_t * out_size)
{
    gzFile gz = gzopen(path, "rb");
    if (!gz) {
        fprintf(stderr, "Error: Cannot open gzip file '%s'\n", path);
        return -1;
    }

    printf("Decompressing gzip file...\n");

    size_t capacity = 64 * 1024; /* Start with 64KB */
    size_t size = 0;
    uint8_t * data = malloc(capacity);
    if (!data) {
        gzclose(gz);
        fprintf(stderr, "Error: Out of memory\n");
        return -1;
    }

    int bytes_read;
    while ((bytes_read = gzread(gz, data + size, capacity - size)) > 0) {
        size += bytes_read;
        if (size == capacity) {
            capacity *= 2;
            uint8_t * new_data = realloc(data, capacity);
            if (!new_data) {
                free(data);
                gzclose(gz);
                fprintf(stderr, "Error: Out of memory\n");
                return -1;
            }
            data = new_data;
        }
    }

    if (bytes_read < 0) {
        int errnum;
        char const * errmsg = gzerror(gz, &errnum);
        fprintf(stderr, "Error: Failed to decompress: %s\n", errmsg);
        free(data);
        gzclose(gz);
        return -1;
    }

    gzclose(gz);
    *out_data = data;
    *out_size = size;
    return 0;
}

static int load_regular(char const * path, uint8_t ** out_data, size_t * out_size)
{
    FILE * fp = fopen(path, "rb");
    if (!fp) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", path);
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (file_size <= 0) {
        fprintf(stderr, "Error: Empty or invalid file\n");
        fclose(fp);
        return -1;
    }

    uint8_t * data = malloc(file_size);
    if (!data) {
        fprintf(stderr, "Error: Out of memory\n");
        fclose(fp);
        return -1;
    }

    if (fread(data, 1, file_size, fp) != (size_t) file_size) {
        fprintf(stderr, "Error: Failed to read file\n");
        free(data);
        fclose(fp);
        return -1;
    }

    fclose(fp);
    *out_data = data;
    *out_size = file_size;
    return 0;
}

int file_load(char const * path, uint8_t ** out_data, size_t * out_size)
{
    if (file_is_gzip(path)) {
        return load_gzip(path, out_data, out_size);
    }
    return load_regular(path, out_data, out_size);
}
