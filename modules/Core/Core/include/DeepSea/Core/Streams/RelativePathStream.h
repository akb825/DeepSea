/*
 * Copyright 2026 Aaron Barany
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

#include <DeepSea/Core/Streams/Types.h>
#include <DeepSea/Core/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for opening streams for relative paths for common stream types.
 */

/**
 * @brief Opens a relative path to a file.
 * @remark errno will be set on failure.
 * @param userData The user data. This is expected to be of type dsFileRelativePath.
 * @param path The relative path to open.
 * @param mode The mode to open the path as.
 * @return The stream or NULL if an error occurred.
 */
DS_CORE_EXPORT dsStream* dsFileRelativePath_open(
	void* userData, const char* path, const char* mode);

/**
 * @brief Closes a stream opened as a relative path.
 * @param userData The user data. This is expected to be of type dsFileRelativePath.
 * @param stream The stream that was opened.
 */
DS_CORE_EXPORT void dsFileRelativePath_close(void* userData, dsStream* stream);

/**
 * @brief Opens a relative path to a resource.
 * @remark errno will be set on failure.
 * @param userData The user data. This is expected to be of type dsResourceRelativePath.
 * @param path The relative path to open.
 * @param mode The mode to open the path as.
 * @return The stream or NULL if an error occurred.
 */
DS_CORE_EXPORT dsStream* dsResourceRelativePath_open(
	void* userData, const char* path, const char* mode);

/**
 * @brief Closes a stream opened as a relative path.
 * @param userData The user data. This is expected to be of type dsResourceRelativePath.
 * @param stream The stream that was opened.
 */
DS_CORE_EXPORT void dsResourceRelativePath_close(void* userData, dsStream* stream);

/**
 * @brief Opens a relative path within an archive.
 * @remark errno will be set on failure.
 * @param userData The user data. This is expected to be of type dsArchiveRelativePath.
 * @param path The relative path to open.
 * @param mode The mode to open the path as.
 * @return The stream or NULL if an error occurred.
 */
DS_CORE_EXPORT dsStream* dsArchiveRelativePath_open(
	void* userData, const char* path, const char* mode);

/**
 * @brief Closes a stream opened as a relative path.
 * @param userData The user data. This is expected to be of type dsArchiveRelativePath.
 * @param stream The stream that was opened.
 */
DS_CORE_EXPORT void dsArchiveRelativePath_close(void* userData, dsStream* stream);
