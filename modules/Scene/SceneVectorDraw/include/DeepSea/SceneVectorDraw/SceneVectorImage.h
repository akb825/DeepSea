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
 * @brief Function for registering dsVectorImage with dsSceneResources.
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
 * @brief Creates a custom resource to wrap a dsVectorImage.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the custom resource.
 * @param vectorImage The vector image to wrap.
 * @return The custom resource or NULL if an error occurred.
 */
DS_SCENEVECTORDRAW_EXPORT dsCustomSceneResource* dsSceneVectorImage_create(dsAllocator* allocator,
	dsVectorImage* vectorImage);

#ifdef __cplusplus
}
#endif
