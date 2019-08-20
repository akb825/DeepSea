/*
 * Copyright 2019 Aaron Barany
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
#include <DeepSea/Scene/ItemLists/Types.h>
#include <DeepSea/Scene/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating dsSceneInstanceData instances that manage instance tansforms.
 *
 * This populates the uniforms found in DeepSea/Scene/Shaders/InstanceTransform.mslh
 *
 * @see dsSceneInstanceData
 */

/**
 * @brief The shader variable group name used for the instance transform data.
 */
DS_SCENE_EXPORT const char* const dsInstanceTransformData_shaderVariableGroupName;

/**
 * @brief Creates the shader variable group description used to describe the variables for the
 *     instance transforms.
 * @remark This should be shared among all dsInstanceTransformData instances.
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager.
 * @param allocator The allocator to create the shader variable group with. If NULL, the allocator
 *     from resourceManager.
 * @return The shader variable group description or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsShaderVariableGroupDesc* dsInstanceTransformData_createShaderVariableGroupDesc(
	dsResourceManager* resourceManager, dsAllocator* allocator);

/**
 * @brief Creates instance trnasform data to use with a dsSceneItemList.
 * @param allocator The allocator to create the transform data with.
 * @param resourceManager The resource manager.
 * @param transformDesc The shader variable group description created from
 *     dsInstanceTransformData_createShaderVariableGroupDesc(). This must remain alive at least as
 *     long as the instance data object.
 * @return The instance data or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsSceneInstanceData* dsInstanceTransformData_create(dsAllocator* allocator,
	dsResourceManager* resourceManager, const dsShaderVariableGroupDesc* transformDesc);

#ifdef __cplusplus
}
#endif
