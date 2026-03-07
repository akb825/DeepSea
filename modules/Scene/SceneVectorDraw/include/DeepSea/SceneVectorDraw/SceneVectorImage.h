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
 * @brief Functions for creating and manipulating scene vector images.
 * @see dsSceneVectorImage
 */

/**
 * @brief The type name for a scene vector image.
 */
DS_SCENEVECTORDRAW_EXPORT extern const char* const dsSceneVectorImage_typeName;

/**
 * @brief Gets the type for the dsVectorImage custom type for storage in dsSceneResources.
 * @return The custom type.
 */
DS_SCENEVECTORDRAW_EXPORT const dsCustomSceneResourceType* dsSceneVectorImage_type(void);

/**
 * @brief Creates a scene vector image object.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the scene text with.
 * @param image The vector image to use within the scene. This will take ownership of the vector
 *     image and destroy it if creation fails.
 * @param shaders The shaders to draw the image with. The shaders must remain alive at least as long
 *     as this object.
 * @return The scene vector image or NULL if an error occurred.
 */
DS_SCENEVECTORDRAW_EXPORT dsSceneVectorImage* dsSceneVectorImage_create(
	dsAllocator* allocator, dsVectorImage* image, const dsVectorShaders* shaders);

/**
 * @brief Destroys a scene vector image.
 * @param image The scene vector image to destroy.
 * @return Whether the scene vector image could be destroyed.
 */
DS_SCENEVECTORDRAW_EXPORT bool dsSceneVectorImage_destroy(dsSceneVectorImage* image);

/**
 * @brief Creates a scene resource to wrap a scene vector image.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the custom resource.
 * @param image The scene vector image to wrap.
 * @return The scene resource or NULL if an error occurred.
 */
DS_SCENEVECTORDRAW_EXPORT dsCustomSceneResource* dsSceneVectorImage_createResource(
	dsAllocator* allocator, dsSceneVectorImage* image);

#ifdef __cplusplus
}
#endif
