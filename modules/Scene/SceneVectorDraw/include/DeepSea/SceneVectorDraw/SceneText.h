/*
 * Copyright 2020-2026 Aaron Barany
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
#include <DeepSea/SceneVectorDraw/Export.h>
#include <DeepSea/SceneVectorDraw/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating scene text.
 * @see dsSceneText
 */

/**
 * @brief Resubstitutes all text within a set of scene resources.
 * @remark errno will be set on failure.
 * @param resources The scene resources to resubstitute text on.
 * @param substitutionTable The substitution table to provide the updated values with.
 * @param substitutionData Temporary data used to perform substitution.
 * @return False if substitution failed.
 */
DS_SCENEVECTORDRAW_EXPORT bool dsSceneText_resubstituteAll(const dsSceneResources* resources,
	const dsTextSubstitutionTable* substitutionTable, dsTextSubstitutionData* substitutionData);

/**
 * @brief The type name for scene text.
 */
DS_SCENEVECTORDRAW_EXPORT extern const char* const dsSceneText_typeName;

/**
 * @brief Gets the type for the dsSceneText custom type for storage in dsSceneResources.
 * @return The custom type.
 */
DS_SCENEVECTORDRAW_EXPORT const dsCustomSceneResourceType* dsSceneText_type(void);

/**
 * @brief Creates a scene text object.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the scene text with.
 * @param font The font to use with the text.
 * @param string The base strnig for the text. This will be copied.
 * @param styles The styles to use with the text.
 * @param styleCount The number of styles.
 * @param substitutionTable The substitution table. If provided and the string requires
 *     substitution, the original text information will be stored to allow later resubstitution.
 *     This may be NULL if no substitution is needed.
 * @param substitutionData Temporary data used to perform substitution. This may be NULL if
 *     substitutionTable is NULL.
 * @return The scene text or NULL if an error occurred.
 */
DS_SCENEVECTORDRAW_EXPORT dsSceneText* dsSceneText_create(dsAllocator* allocator, dsFont* font,
	const char* string, const dsTextStyle* styles, uint32_t styleCount,
	const dsTextSubstitutionTable* substitutionTable, dsTextSubstitutionData* substitutionData);

/**
 * @brief Performs substitution on scene text with the current values of a substitution table.
 *
 * This will increment the text version, indicating that any text nodes utilizing the text should
 * have its layout re-computed.
 *
 * @remark errno will be set on failure.
 * @param text The text to perform substitution on.
 * @param substitutionTable The substitution table to provide the updated values with.
 * @param substitutionData Temporary data used to perform substitution.
 * @return False if substitution failed.
 */
DS_SCENEVECTORDRAW_EXPORT bool dsSceneText_resubstitute(dsSceneText* text,
	const dsTextSubstitutionTable* substitutionTable, dsTextSubstitutionData* substitutionData);

/**
 * @brief Destroys a scene text.
 * @param text The text to destroy.
 */
DS_SCENEVECTORDRAW_EXPORT void dsSceneText_destroy(dsSceneText* text);

/**
 * @brief Creates a scene resource to wrap a scene text.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the resource with.
 * @param text The scene text to wrap. This will take ownership of the text and destory it if
 *     creation fails.
 * @return The scene resource or NULL if an error occurred.
 */
DS_SCENEVECTORDRAW_EXPORT dsCustomSceneResource* dsSceneText_createResource(
	dsAllocator* allocator, dsSceneText* text);

#ifdef __cplusplus
}
#endif
