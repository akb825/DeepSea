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
#include <DeepSea/Scene/ItemLists/Types.h>
#include <DeepSea/SceneVectorDraw/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating dsSceneInstanceData instances that manage instance discard bounds.
 *
 * This populates the uniforms found in DeepSea/SceneVectorDraw/Shaders/InstanceDiscardBounds.mslh
 *
 * @see dsSceneInstanceData
 */

/**
 * @brief The instance discard bounds data type name.
 */
DS_SCENEVECTORDRAW_EXPORT extern const char* const dsInstanceDiscardBoundsData_typeName;

/**
 * @brief The instance discard bounds data shader uniform name.
 */
DS_SCENEVECTORDRAW_EXPORT extern const char* const dsInstanceDiscardBoundsData_uniformName;

/**
 * @brief Creates the shader variable group description used to describe the variables for instance
 *     discard bounds.
 * @remark This should be shared among all dsInstanceDiscardBoundsData instances.
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager.
 * @param allocator The allocator to create the shader variable group with. If NULL, the allocator
 *     from resourceManager.
 * @return The shader variable group description or NULL if an error occurred.
 */
DS_SCENEVECTORDRAW_EXPORT dsShaderVariableGroupDesc*
dsInstanceDiscardBoundsData_createShaderVariableGroupDesc(
	dsResourceManager* resourceManager, dsAllocator* allocator);

/**
 * @brief Checks whether or not a shader variable group is compatible with
 *     dsInstanceDiscardBoundsData.
 * @param discardBoundsDesc The shader variable group for the transform.
 * @return Whether or not discardBoundsDesc is compatible.
 */
DS_SCENEVECTORDRAW_EXPORT bool dsInstanceDiscardBoundsData_isShaderVariableGroupCompatible(
	const dsShaderVariableGroupDesc* discardBoundsDesc);

/**
 * @brief Creates instance discard bounds data to use with a dsSceneItemList.
 * @param allocator The allocator to create the transform data with. This must support freeing
 *     memory.
 * @param resourceAllocator The allocator to create graphics resources with. If NULL this will
 *     default to allocator.
 * @param resourceManager The resource manager.
 * @param transformDesc The shader variable group description created from
 *     dsInstanceDiscardBoundsData_createShaderVariableGroupDesc(). This must remain alive at least
 *     as long as the instance data object.
 * @return The instance data or NULL if an error occurred.
 */
DS_SCENEVECTORDRAW_EXPORT dsSceneInstanceData* dsInstanceDiscardBoundsData_create(
	dsAllocator* allocator, dsAllocator* resourceAllocator, dsResourceManager* resourceManager,
	const dsShaderVariableGroupDesc* transformDesc);

#ifdef __cplusplus
}
#endif
