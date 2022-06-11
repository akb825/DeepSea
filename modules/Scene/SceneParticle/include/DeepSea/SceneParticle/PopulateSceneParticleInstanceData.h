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
#include <DeepSea/SceneParticle/Export.h>
#include <DeepSea/SceneParticle/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Function to populate instance data for a particle scene emitter in a scene.
 * @see dsSceneParticleInstanceData
 */

/**
 * @brief Populates the instance data for particles in a scene.
 * @remark errno will be set on failure.
 * @param emitter The particle emitter.
 * @param userData The user data. This must be of type dsSceneTreeNode.
 * @param values The values to populate.
 * @param drawData The draw data. This must be of type dsSceneParticleInstanceData.
 * @return False if the values couldn't be set.
 */
DS_SCENEPARTICLE_EXPORT bool dsPopulateSceneParticleInstanceData(const dsParticleEmitter* emitter,
	void* userData, dsSharedMaterialValues* values, void* drawData);

#ifdef __cplusplus
}
#endif
