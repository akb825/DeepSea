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
#include <errno.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Function to get a string for an error code.
 */

/**
 * @brief Gets the string for an error number.
 *
 * This is thread-safe, using a statically allocated thread-local buffer for the string.
 *
 * @param errorCode The error code to get the string for. This will typically be errno.
 * @return A string for the error number.
 */
DS_CORE_EXPORT const char* dsErrorString(int errorCode);

#ifdef __cplusplus
}
#endif
