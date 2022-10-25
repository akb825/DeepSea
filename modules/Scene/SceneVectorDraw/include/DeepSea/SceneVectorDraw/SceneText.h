/*
 * Copyright 2020-2022 Aaron Barany
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
 * @param text The text to use within the scene. This will take ownership of the text and destroy
 *     it if creation fails.
 * @param userData User data to store with the text.
 * @param styles The styles to use with the text.
 * @param styleCount The number of styles.
 * @return The scene text or NULL if an error occurred.
 */
DS_SCENEVECTORDRAW_EXPORT dsSceneText* dsSceneText_create(dsAllocator* allocator, dsText* text,
	void* userData, const dsTextStyle* styles, uint32_t styleCount);

/**
 * @brief Destroys a scene text.
 * @param text The text to destroy.
 */
DS_SCENEVECTORDRAW_EXPORT void dsSceneText_destroy(dsSceneText* text);

/**
 * @brief Creates a scene resource to wrap a scene text.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the resource with.
 * @param text The scene text to wrap.
 * @return The scene resource or NULL if an error occurred.
 */
DS_SCENEVECTORDRAW_EXPORT dsCustomSceneResource* dsSceneText_createResource(dsAllocator* allocator,
	dsSceneText* text);

#ifdef __cplusplus
}
#endif
