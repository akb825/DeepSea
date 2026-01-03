/*
 * Copyright 2019-2026 Aaron Barany
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
#include <DeepSea/Scene/Export.h>
#include <DeepSea/Scene/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating dsSceneItemList instances that manage view tansforms.
 *
 * This populates the uniforms found in DeepSea/Scene/Shaders/ViewTransform.mslh
 *
 * @remark This item list type contains global values and must be a alone as an entry in the
 *     sharedItemlist of a dsScene.
 */

/**
 * @brief The view transform data type name.
 */
DS_SCENE_EXPORT extern const char* const dsViewTransformData_typeName;

/**
 * @brief The view transform data shader uniform name.
 */
DS_SCENE_EXPORT extern const char* const dsViewTransformData_uniformName;

/**
 * @brief Gets the type of a view transform data.
 * @return The type of a view transform data.
 */
DS_SCENE_EXPORT const dsSceneItemListType* dsViewTransformData_type(void);

/**
 * @brief Creates the shader variable group description used to describe the variables for the
 *     view transforms.
 * @remark This should be shared among all dsViewTransformData instances.
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager.
 * @param allocator The allocator to create the shader variable group with. If NULL, the allocator
 *     from resourceManager.
 * @return The shader variable group description or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsShaderVariableGroupDesc* dsViewTransformData_createShaderVariableGroupDesc(
	dsResourceManager* resourceManager, dsAllocator* allocator);

/**
 * @brief Creates view trnasform data to use with a dsScene.
 * @param allocator The allocator to create the transform data with.
 * @param name The name of the light set prepare. This will be copied.
 * @param resourceManager The resource manager.
 * @param transformDesc The shader variable group description created from
 *     dsViewTransformData_createShaderVariableGroupDesc(). This must remain alive at least as
 *     long as the instance data object.
 * @return The global data or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsSceneItemList* dsViewTransformData_create(dsAllocator* allocator,
	const char* name, dsResourceManager* resourceManager,
	const dsShaderVariableGroupDesc* transformDesc);

#ifdef __cplusplus
}
#endif

