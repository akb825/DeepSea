/*
 * Copyright 2020 Aaron Barany
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

#include <DeepSea/Core/Config.h>
#include <DeepSea/Text/Export.h>
#include <DeepSea/Text/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Function for maintaining a substitution table and substituting strings.
 * @see dsTextSubstitutionTable
 */

/**
 * @brief Creates a substitution table.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the substitution table with. This must support freeing
 *     memory.
 * @param maxStrings The maximum number of strings for substitution.
 * @return The substitution table or NULL if an error occurred.
 */
DS_TEXT_EXPORT dsTextSubstitutionTable* dsTextSubstitutionTable_create(dsAllocator* allocator,
	uint32_t maxStrings);

/**
 * @brief Gets the remaining strings that can be added to the substitution table.
 * @param table The subtitation table.
 * @return The number of remaining strings that can be added.
 */
DS_TEXT_EXPORT uint32_t dsTextSubstitutionTable_getRemainingStrings(
	const dsTextSubstitutionTable* table);

/**
 * @brief Sets a string to the substitution table.
 *
 * This can either add a new string or replace a previously set string.
 *
 * @remark errno will be set on failure.
 * @param table The substitution table to add the string to.
 * @param name The name of the variable. This will be copied.
 * @param value The value for the string. This will be copied.
 * @return False if the string couldn't be added.
 */
DS_TEXT_EXPORT bool dsTextSubstitutionTable_setString(dsTextSubstitutionTable* table,
	const char* name, const char* value);

/**
 * @brief Gets a string by its name.
 * @param table The substitution table to get the string from.
 * @param name The name of the string to get.
 * @return The string value or NULL if not found.
 */
DS_TEXT_EXPORT const char* dsTextSubstitutionTable_getString(dsTextSubstitutionTable* table,
	const char* name);

/**
 * @brief Removes a string from the substitution table.
 * @param table The substitution table to remove the string from.
 * @param name The name of the string to remove.
 * @return False if the string couldn't be removed.
 */
DS_TEXT_EXPORT bool dsTextSubstitutionTable_removeString(dsTextSubstitutionTable* table,
	const char* name);

/**
 * @brief Substitutes a string based on the substitution table.
 *
 * Any substring with the pattern ${x} will search for the string named "x" to be substituted. It is
 * an error if the name inside of the ${} is not found.
 *
 * @remark errno will be set on failure.
 * @param table The substitution table to perform the substitution.
 * @param data The data to hold the temporary string data. The returned data will remain valid until
 *     this data is destroyed or used for another call to substitute. It is suggested to re-use the
 *     same data instance as much as possible to avoid re-allocations.
 * @param string The string to substitute.
 * @param ranges The style ranges to adjust based on the substitutions.
 * @param rangeCount The number of ranges.
 */
DS_TEXT_EXPORT const char* dsTextSubstitutionTable_substitute(const dsTextSubstitutionTable* table,
	dsTextSubstitutionData* data, const char* string, dsTextStyle* ranges, uint32_t rangeCount);

/**
 * @brief Destroys a substitution table.
 * @param table The substitution table to destroy.
 */
DS_TEXT_EXPORT void dsTextSubstitutionTable_destroy(dsTextSubstitutionTable* table);

#ifdef __cplusplus
}
#endif
