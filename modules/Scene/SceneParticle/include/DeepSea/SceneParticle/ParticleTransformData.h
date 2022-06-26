/*
 * Copyright 2022 Aaron Barany
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
 * This populates the uniforms found in DeepSea/Scene/Shaders/ParticleTransform.mslh
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
 * @brief Creates particle trnasform data to use with a dsSceneItemList.
 * @param allocator The allocator to create the transform data with.
 * @param resourceManager The resource manager.
 * @param transformDesc The shader variable group description created from
 *     dsInstanceTransformData_createShaderVariableGroupDesc(). This must remain alive at least as
 *     long as the instance data object.
 * @return The instance data or NULL if an error occurred.
 */
DS_SCENEPARTICLE_EXPORT dsSceneInstanceData* dsParticleTransformData_create(dsAllocator* allocator,
	dsResourceManager* resourceManager, const dsShaderVariableGroupDesc* transformDesc);

#ifdef __cplusplus
}
#endif
