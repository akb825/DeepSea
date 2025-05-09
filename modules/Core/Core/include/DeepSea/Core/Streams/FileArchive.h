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
 * @brief Functions for operating on file archives.
 * @see dsFileArchive
 */

/**
 * @brief Gets the status on a path within the archive.
 * @remark errno will be set on failure.
 * @param archive The archive to get the path status from.
 * @param path The path within the archive.
 * @return The path status.
 */
DS_CORE_EXPORT dsPathStatus dsFileArchive_pathStatus(
	const dsFileArchive* archive, const char* path);

/**
 * @brief Opens a directory within an archive.
 * @remark errno will be set on failure.
 * @param archive The archive to open the directory from.
 * @param path The path to the directory within the archive.
 * @return The directory iterator or NULL if the directory cannot be iterated.
 */
DS_CORE_EXPORT dsDirectoryIterator dsFileArchive_openDirectory(
	const dsFileArchive* archive, const char* path);

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
DS_CORE_EXPORT dsPathStatus dsFileArchive_nextDirectoryEntry(
	char* result, size_t resultSize, const dsFileArchive* archive, dsDirectoryIterator iterator);

/**
 * @brief Closes a directory within an archive.
 * @remark errno will be set on failure.
 * @param archive The archive the directory was opened with.
 * @param iterator The iterator for the directory to close.
 * @return False if the directory couldn't be closed.
 */
DS_CORE_EXPORT bool dsFileArchive_closeDirectory(
	const dsFileArchive* archive, dsDirectoryIterator iterator);

/**
 * @brief Opens a file within an archive.
 *
 * The stream will typically be dynamically allocated, and will be freed once dsStream_close() is
 * called.
 *
 * @remark errno will be set on failure.
 * @param archive The archive to open the file with.
 * @param path The path to the file to open.
 * @return The opened stream or NULL if the file couldn't be opened.
 */
DS_CORE_EXPORT dsStream* dsFileArchive_openFile(const dsFileArchive* archive, const char* path);

/**
 * @brief Closes an archive, freeing all associated resources.
 *
 * All files and directories should be closed before closing the archive itself.
 *
 * @param archive The archive to close.
 */
DS_CORE_EXPORT void dsFileArchive_close(dsFileArchive* archive);

#ifdef __cplusplus
}
#endif
