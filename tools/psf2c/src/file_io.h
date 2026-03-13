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
 * @file file_io.h
 * @brief File loading utilities with gzip support.
 */

#ifndef FILE_IO_H
#define FILE_IO_H

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Check if a file is gzip compressed.
 *
 * @param path Path to the file
 * @return Non-zero if gzip compressed, 0 otherwise
 */
int file_is_gzip(char const * path);

/**
 * @brief Load a file into memory.
 *
 * Automatically handles gzip-compressed files. The caller is responsible
 * for freeing the returned data with free().
 *
 * @param path      Path to the file
 * @param out_data  Output pointer to file data
 * @param out_size  Output size in bytes
 * @return 0 on success, -1 on error
 */
int file_load(char const * path, uint8_t ** out_data, size_t * out_size);

#endif /* FILE_IO_H */
