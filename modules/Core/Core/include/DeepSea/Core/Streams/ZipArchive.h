/*
 * Copyright 2025 Aaron Barany
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Core/Export.h>
#include <DeepSea/Core/Streams/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating zip archives.
 *
 * All paths within a zip archive will be rleative to the root of the archive. A leading ./ may be
 * used for any path, including using "." by itself to refer to the root directory of the archive.
 *
 * @see dsZipArchive
 */

/**
 * @brief Opens a zip archive from a file path.
 * @remark errno will be set on failure.
 * @param allocator The allocator for memory associated with the archive. This must support freeing
 *     memory.
 * @param path The path to the zip archive.
 * @param decompressBufferSize The size of the buffer when reading compressed data. Set to 0 to use
 *     the default.
 * @return The zip archive or NULL if it couldn't be opened.
 */
DS_CORE_EXPORT dsZipArchive* dsZipArchive_open(
	dsAllocator* allocator, const char* path, size_t decompressBufferSize);

/**
 * @brief Opens a zip archive from a resource path.
 * @remark errno will be set on failure.
 * @param allocator The allocator for memory associated with the archive. This must support freeing
 *     memory.
 * @param type The resource type.
 * @param path The path to the zip archive.
 * @param decompressBufferSize The size of the buffer when reading compressed data. Set to 0 to use
 *     the default.
 * @return The zip archive or NULL if it couldn't be opened.
 */
DS_CORE_EXPORT dsZipArchive* dsZipArchive_openResource(
	dsAllocator* allocator, dsFileResourceType type, const char* path, size_t decompressBufferSize);

/**
 * @brief Gets the status on a path within the archive.
 * @remark errno will be set on failure.
 * @param archive The archive to get the path status from.
 * @param path The path within the archive.
 * @return The path status.
 */
DS_CORE_EXPORT dsPathStatus dsZipArchive_pathStatus(
	const dsZipArchive* archive, const char* path);

/**
 * @brief Opens a directory within an archive.
 * @remark errno will be set on failure.
 * @param archive The archive to open the directory from.
 * @param path The path to the directory within the archive.
 * @return The directory iterator or NULL if the directory cannot be iterated.
 */
DS_CORE_EXPORT dsDirectoryIterator dsZipArchive_openDirectory(
	const dsZipArchive* archive, const char* path);

/**
 * @brief Gets the next entry within a directory in an archive.
 * @remark errno will be set on failure.
 * @param[out] result The storage for the result.
 * @param resultSize The maximum size of the result.
 * @param archive The archive the directory was opened with.
 * @param iterator The iterator for the directory.
 * @return The result of getting the next entry. dsPathStatus_Missing will be returned once the last
 *     entry has been reached.
 */
DS_CORE_EXPORT dsPathStatus dsZipArchive_nextDirectoryEntry(
	char* result, size_t resultSize, const dsZipArchive* archive, dsDirectoryIterator iterator);

/**
 * @brief Closes a directory within an archive.
 * @remark errno will be set on failure.
 * @param archive The archive the directory was opened with.
 * @param iterator The iterator for the directory to close.
 * @return False if the directory couldn't be closed.
 */
DS_CORE_EXPORT bool dsZipArchive_closeDirectory(
	const dsZipArchive* archive, dsDirectoryIterator iterator);

/**
 * @brief Opens a file within an archive.
 * @remark errno will be set on failure.
 * @param archive The archive to open the file with.
 * @param path The path to the file to open.
 * @return The opened stream or NULL if the file couldn't be opened.
 */
DS_CORE_EXPORT dsStream* dsZipArchive_openFile(const dsZipArchive* archive, const char* path);

/**
 * @brief Closes a file within an archive.
 * @remark errno will be set on failure.
 * @param archive The archive the file was opened with.
 * @param stream The stream for the file that was oepend.
 * @return False if the file couldn't be closed.
 */
DS_CORE_EXPORT bool dsZipArchive_closeFile(const dsZipArchive* archive, dsStream* stream);

/**
 * @brief Closes a zip archive.
 *
 * All files and directories must be closed before calling this function.
 *
 * @param archive The archive to close.
 */
DS_CORE_EXPORT void dsZipArchive_close(dsZipArchive* archive);

#ifdef __cplusplus
}
#endif
