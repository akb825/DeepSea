/*
 * Copyright 2022-2023 Aaron Barany
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
#include <DeepSea/SceneParticle/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating dsSceneInstanceData instances that manage particle transforms.
 *
 * This populates the uniforms found in DeepSea/SceneParticle/Shaders/ParticleTransform.mslh
 *
 * This is similar to instance transform data, except it uses the transform used by the particle
 * emitter. In some cases this transform will be based off a node different from the node that
 * manages it.
 *
 * @see dsSceneInstanceData
 */

/**
 * @brief The particle transform data type name.
 */
DS_SCENEPARTICLE_EXPORT extern const char* const dsParticleTransformData_typeName;

/**
 * @brief Creates the shader variable group description used to describe the variables for particle
 *     transforms.
 * @remark This should be shared among all dsParticleTransformData instances.
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager.
 * @param allocator The allocator to create the shader variable group with. If NULL, the allocator
 *     from resourceManager.
 * @return The shader variable group description or NULL if an error occurred.
 */
DS_SCENEPARTICLE_EXPORT dsShaderVariableGroupDesc*
dsParticleTransformData_createShaderVariableGroupDesc(dsResourceManager* resourceManager,
	dsAllocator* allocator);

/**
 * @brief Checks whether or not a shader variable group is compatible with dsParticleTransformData.
 * @param transformDesc The shader variable group for the transform.
 * @return Whether or not transformDesc is compatible.
 */
DS_SCENEPARTICLE_EXPORT bool dsParticleTransformData_isShaderVariableGroupCompatible(
	const dsShaderVariableGroupDesc* transformDesc);

/**
 * @brief Creates particle trnasform data to use with a dsSceneItemList.
 * @param allocator The allocator to create the transform data with. This must support freeing
 *     memory.
 * @param resourceAllocator The allocator to create graphics resources with. If NULL this will
 *     default to allocator.
 * @param resourceManager The resource manager.
 * @param transformDesc The shader variable group description created from
 *     dsParticleTransformData_createShaderVariableGroupDesc(). This must remain alive at least as
 *     long as the instance data object.
 * @return The instance data or NULL if an error occurred.
 */
DS_SCENEPARTICLE_EXPORT dsSceneInstanceData* dsParticleTransformData_create(dsAllocator* allocator,
	dsAllocator* resourceAllocator, dsResourceManager* resourceManager,
	const dsShaderVariableGroupDesc* transformDesc);

#ifdef __cplusplus
}
#endif
