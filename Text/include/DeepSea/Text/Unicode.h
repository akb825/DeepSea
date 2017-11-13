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
 * @brief Gets the length of a UTF-8 string in characters.
 * @param string The UTF-8 string.
 * @return The length of the string.
 */
DS_TEXT_EXPORT uint32_t dsUTF8_length(const char* string);

/**
 * @brief Gets the size of a codepoint in UTF-8.
 * @param codepoint The codepoint.
 * @return The size of the codepoint in characters or DS_UNICODE_INVALID if the codepoint is
 *     invalid.
 */
DS_TEXT_EXPORT uint32_t dsUTF8_codepointSize(uint32_t codepoint);

/**
 * @brief Adds a codepoint to a UTF-8 string.
 * @param string The string to add the codepoint to.
 * @param length The length available in the string.
 * @param offset The offset into the string to add the codepoint.
 * @param codepoint The codepoint to add.
 * @return The new offset into the string, or DS_UNICODE_INVALID if the codepoint is invalid or
 *     there is no space available.
 */
DS_TEXT_EXPORT uint32_t dsUTF8_addCodepoint(char* string, uint32_t length, uint32_t offset,
	uint32_t codepoint);

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

/**
 * @brief Gets the length of a UTF-16 string in characters.
 * @param string The UTF-16 string.
 * @return The length of the string.
 */
DS_TEXT_EXPORT uint32_t dsUTF16_length(const uint16_t* string);

/**
 * @brief Gets the size of a codepoint in UTF-16.
 * @param codepoint The codepoint.
 * @return The size of the codepoint in characters or DS_UNICODE_INVALID if the codepoint is
 *     invalid.
 */
DS_TEXT_EXPORT uint32_t dsUTF16_codepointSize(uint32_t codepoint);

/**
 * @brief Adds a codepoint to a UTF-16 string.
 * @param string The string to add the codepoint to.
 * @param length The length available in the string.
 * @param offset The offset into the string to add the codepoint.
 * @param codepoint The codepoint to add.
 * @return The new offset into the string, or DS_UNICODE_INVALID if the codepoint is invalid or
 *     there is no space available.
 */
DS_TEXT_EXPORT uint32_t dsUTF16_addCodepoint(uint16_t* string, uint32_t length, uint32_t offset,
	uint32_t codepoint);

/**
 * @brief Gets the next codepoint in a UTF-32 string.
 * @param string The UTF-32 string.
 * @param[inout] index The current index into the string. This will be updated to point to the next
 *     codepoint.
 * @return The codepoint.
 */
DS_TEXT_EXPORT uint32_t dsUTF32_nextCodepoint(const uint32_t* string, uint32_t* index);

/**
 * @brief Gets the number of codepoints in a UTF-32 string.
 * @param string The UTF-32 string.
 * @return The number of codepoints, or DS_UNICODE_INVALID if the string is invalid.
 */
DS_TEXT_EXPORT uint32_t dsUTF32_codepointCount(const uint32_t* string);

/**
 * @brief Gets the length of a UTF-32 string in characters.
 * @param string The UTF-32 string.
 * @return The length of the string.
 */
DS_TEXT_EXPORT uint32_t dsUTF32_length(const uint32_t* string);

/**
 * @brief Gets the size of a codepoint in UTF-32.
 * @param codepoint The codepoint.
 * @return The size of the codepoint in characters, which will always be 1.
 */
DS_TEXT_EXPORT uint32_t dsUTF32_codepointSize(uint32_t codepoint);

/**
 * @brief Adds a codepoint to a UTF-32 string.
 * @param string The string to add the codepoint to.
 * @param length The length available in the string.
 * @param offset The offset into the string to add the codepoint.
 * @param codepoint The codepoint to add.
 * @return The new offset into the string, or DS_UNICODE_INVALID if there is no space available.
 */
DS_TEXT_EXPORT uint32_t dsUTF32_addCodepoint(uint32_t* string, uint32_t length, uint32_t offset,
	uint32_t codepoint);

#ifdef __cplusplus
}
#endif
