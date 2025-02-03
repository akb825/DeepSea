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

#if DS_ANDROID

#include <jni.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating Android archives, operating on an
 * AssetManager.
 * @see dsAndroidArchive
 */

/**
 * @brief Opens an Android archive from an AssetManager.
 * @remark errno will be set on failure.
 * @param allocator The allocator for memory associated with the archive. This must support freeing
 *     memory.
 * @param env The Java environment for JNI calls.
 * @param assetManage The Java asset manager.
 * @return The android archive or NULL if it couldn't be opened.
 */
DS_CORE_EXPORT dsAndroidArchive* dsAndroidArchive_open(
	dsAllocator* allocator, JNIEnv* env, jobject assetManager);

/**
 * @brief Gets the status on a path within the archive.
 * @rmeark Due to limitations with Android's AssetManager implementation, a path that doesn't exist
 *     may be incorrectly detected as an existing directory.
 * @remark errno will be set on failure.
 * @param archive The archive to get the path status from.
 * @param path The path within the archive.
 * @return The path status.
 */
DS_CORE_EXPORT dsPathStatus dsAndroidArchive_pathStatus(
	const dsAndroidArchive* archive, const char* path);

/**
 * @brief Opens a directory within an archive.
 * @rmark Due to limitations with Android's AssetManager implementation, directories that don't
 *     exist may appear to have no error.
 * @remark errno will be set on failure.
 * @param archive The archive to open the directory from.
 * @param path The path to the directory within the archive.
 * @return The directory iterator or NULL if the directory cannot be iterated.
 */
DS_CORE_EXPORT dsDirectoryIterator dsAndroidArchive_openDirectory(
	const dsAndroidArchive* archive, const char* path);

/**
 * @brief Gets the next entry within a directory in an archive.
 * @remark Due to limitations with Android's AssetManager implementation, only files will be
 *     returned, and directories will be skipped.
 * @remark errno will be set on failure.
 * @param[out] result The storage for the result.
 * @param resultSize The maximum size of the result.
 * @param archive The archive the directory was opened with.
 * @param iterator The iterator for the directory.
 * @return The result of getting the next entry. dsPathStatus_Missing will be returned once the last
 *     entry has been reached.
 */
DS_CORE_EXPORT dsPathStatus dsAndroidArchive_nextDirectoryEntry(
	char* result, size_t resultSize, const dsAndroidArchive* archive, dsDirectoryIterator iterator);

/**
 * @brief Closes a directory within an archive.
 * @remark errno will be set on failure.
 * @param archive The archive the directory was opened with.
 * @param iterator The iterator for the directory to close.
 * @return False if the directory couldn't be closed.
 */
DS_CORE_EXPORT bool dsAndroidArchive_closeDirectory(
	const dsAndroidArchive* archive, dsDirectoryIterator iterator);

/**
 * @brief Opens a file within an archive.
 *
 * The stream will be dynamically allocated, and will be freed once dsStream_close() is called.
 *
 * @remark errno will be set on failure.
 * @param archive The archive to open the file with.
 * @param path The path to the file to open.
 * @return The opened stream or NULL if the file couldn't be opened.
 */
DS_CORE_EXPORT dsStream* dsAndroidArchive_openFile(
	const dsAndroidArchive* archive, const char* path);

/**
 * @brief Closes a android archive.
 *
 * All files and directories must be closed before calling this function.
 *
 * @param archive The archive to close.
 */
DS_CORE_EXPORT void dsAndroidArchive_close(dsAndroidArchive* archive);

#ifdef __cplusplus
}
#endif

#endif
