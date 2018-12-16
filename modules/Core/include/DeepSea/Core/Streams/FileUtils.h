/*
 * Copyright 2018 Aaron Barany
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
 * @brief Utility functions for manipulating files.
 */

/**
 * @brief Creates a directory.
 * @remark errno will be set on failure.
 * @param dirName The name of the directory.
 * @return False if the directory couldn't be created. If errno is EEXIST, the directory already
 *     existed.
 */
DS_CORE_EXPORT bool dsCreateDirectory(const char* dirName);

/**
 * @brief Gets the status of a file or directory.
 * @remark errno will be set on failure.
 * @param fileName The name of the file or directory.
 * @return The status of the file.
 */
DS_CORE_EXPORT dsFileStatus dsGetFileStatus(const char* fileName);

#ifdef __cplusplus
}
#endif
