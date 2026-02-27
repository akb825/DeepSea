/*
 * Copyright 2026 Aaron Barany
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
 * @brief Functions to create view framebuffer data.
 */

/**
 * @brief The view framebuffer data type name.
 */
DS_SCENE_EXPORT extern const char* const dsViewFramebufferData_typeName;

/**
 * @brief The view framebuffer data shader uniform name.
 */
DS_SCENE_EXPORT extern const char* const dsViewFramebufferData_uniformName;

/**
 * @brief Gets the type of view framebuffer data.
 * @return The type of view framebuffer data.
 */
DS_SCENE_EXPORT const dsSceneInstanceDataType* dsViewFramebufferData_type(void);

/**
 * @brief Creates the shader variable group description used to describe the variables for view
 *     framebuffers.
 * @remark This should be shared among all dsViewFramebufferData instances.
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager.
 * @param allocator The allocator to create the shader variable group with. If NULL, the allocator
 *     from resourceManager.
 * @return The shader variable group description or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsShaderVariableGroupDesc* dsViewFramebufferData_createShaderVariableGroupDesc(
	dsResourceManager* resourceManager, dsAllocator* allocator);

/**
 * @brief Checks whether or not a shader variable group is compatible with dsViewFramebufferData.
 * @param transformDesc The shader variable group for the transform.
 * @return Whether or not transformDesc is compatible.
 */
DS_SCENE_EXPORT bool dsViewFramebufferData_isShaderVariableGroupCompatible(
	const dsShaderVariableGroupDesc* transformDesc);

/**
 * @brief Creates view framebuffer data to use with a dsSceneItemList.
 * @param allocator The allocator to create the transform data with. This must support freeing
 *     memory.
 * @param resourceManager The resource manager.
 * @param resourceAllocator The allocator to create graphics resources with. If NULL this will
 *     default to allocator.
 * @param dataDesc The shader variable group description created from
 *     dsViewFramebufferData_createShaderVariableGroupDesc(). This must remain alive at least as
 *     long as the instance data object.
 * @return The instance data or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsSceneInstanceData* dsViewFramebufferData_create(dsAllocator* allocator,
	dsResourceManager* resourceManager, dsAllocator* resourceAllocator,
	const dsShaderVariableGroupDesc* dataDesc);

#ifdef __cplusplus
}
#endif
