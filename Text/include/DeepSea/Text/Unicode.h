/*
 * Copyright 2017 Aaron Barany
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
#include <DeepSea/Text/Export.h>
#include <DeepSea/Text/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief File containing unicode conversion functions.
 */

/**
 * @brief Constant for the code point at the end of the string.
 */
#define DS_UNICODE_END 0

/**
 * @brief Constant for an invalid character sequence.
 */
#define DS_UNICODE_INVALID 0xFFFFFFFF

/**
 * @brief Gets the next codepoint in a UTF-8 string.
 * @param string The UTF-8 string.
 * @param[inout] index The current index into the string. This will be updated to point to the next
 *     codepoint.
 * @return The codepoint.
 */
DS_TEXT_EXPORT uint32_t dsUTF8_nextCodepoint(const char* string, uint32_t* index);

/**
 * @brief Gets the number of codepoints in a UTF-8 string.
 * @param string The UTF-8 string.
 * @return The number of codepoints, or DS_UNICODE_INVALID if the string is invalid.
 */
DS_TEXT_EXPORT uint32_t dsUTF8_codepointCount(const char* string);

/**
 * @brief Gets the next codepoint in a UTF-16 string.
 * @param string The UTF-16 string.
 * @param[inout] index The current index into the string. This will be updated to point to the next
 *     codepoint.
 * @return The codepoint.
 */
DS_TEXT_EXPORT uint32_t dsUTF16_nextCodepoint(const uint16_t* string, uint32_t* index);

/**
 * @brief Gets the number of codepoints in a UTF-16 string.
 * @param string The UTF-18 string.
 * @return The number of codepoints, or DS_UNICODE_INVALID if the string is invalid.
 */
DS_TEXT_EXPORT uint32_t dsUTF16_codepointCount(const uint16_t* string);

#ifdef __cplusplus
}
#endif
