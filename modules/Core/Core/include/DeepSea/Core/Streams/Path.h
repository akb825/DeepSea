/*
 * Copyright 2016 Aaron Barany
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
 * @brief Helper functions for manipulating paths.
 *
 * For each of the functions that take a result buffer as a parameter, it is safe to have it be the
 * same as the input path. The exception is dsPath_combine(), where the buffer cannot be the same as
 * path2. (but it can be path1)
 */

/**
 * @brief Combines two paths.
 * @remark errno will be set on failure.
 * @param[out] result The result to place the combined path into. This cannot be the same as path2.
 * @param resultSize The size of the result buffer, including the null terminator. This should
 *     contains pace for both path1 and path2, plus the path separator. (if not already at the end
 *     of path 2)
 * @param path1 The first path.
 * @param path2 The second path to add to the first one.
 * @return False if the parameters are invalid or there isn't enough space in result.
 */
DS_CORE_EXPORT bool dsPath_combine(char* result, size_t resultSize, const char* path1,
	const char* path2);

/**
 * @brief Returns whether or not a path is absolute.
 * @return True if the path is absolute, false if it's relative.
 */
DS_CORE_EXPORT bool dsPath_isAbsolute(const char* path);

/**
 * @brief Gets the directory name of a path.
 * @remark errno will be set on failure.
 * @param[out] result The result to place the directory name into.
 * @param resultSize The size of the result buffer, including the null terminator.
 * @param path The path to get the directory name from.
 * @return False if the parameters are invalid or there isn't enough space in result.
 */
DS_CORE_EXPORT bool dsPath_getDirectoryName(char* result, size_t resultSize, const char* path);

/**
 * @brief Gets the file name of the path.
 * @param path The path to get the file name for.
 * @return The file name.
 */
DS_CORE_EXPORT const char* dsPath_getFileName(const char* path);

/**
 * @brief Gets the extension of the path.
 * @param path The path to get the extension from.
 * @return The extension, or NULL if there's no extension. This will include the '.'.
 */
DS_CORE_EXPORT const char* dsPath_getExtension(const char* path);

/**
 * @brief Gets the last extension of the path.
 * @param path The path to get the extension from.
 * @return The last extension, or NULL if there's no extension. This will include the '.'.
 */
DS_CORE_EXPORT const char* dsPath_getLastExtension(const char* path);

/**
 * @brief Removes the last extension on a path.
 * @remark errno will be set on failure.
 * @param[out] result The result to place the path into.
 * @param resultSize The size of the result buffer, including the null terminator.
 * @param path The path to remove the file name.
 * @return False if the parameters are invalid or there isn't enough space in result.
 */
DS_CORE_EXPORT bool dsPath_removeLastExtension(char* result, size_t resultSize, const char* path);

#ifdef __cplusplus
}
#endif
