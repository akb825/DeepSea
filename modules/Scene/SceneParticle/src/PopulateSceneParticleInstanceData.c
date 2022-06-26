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

#include <DeepSea/SceneParticle/PopulateSceneParticleInstanceData.h>

#include <DeepSea/Core/Error.h>
#include <DeepSea/Scene/ItemLists/SceneInstanceData.h>

bool dsPopulateSceneParticleInstanceData(const dsParticleEmitter* emitter,
	void* userData, dsSharedMaterialValues* values, uint32_t index, void* drawData)
{
	if (!emitter || !userData || !values || !drawData)
	{
		errno = EINVAL;
		return false;
	}

	const dsSceneParticleInstanceData* instanceData = (const dsSceneParticleInstanceData*)drawData;
	for (uint32_t i = 0; i < instanceData->instanceCount; ++i)
	{
		if (!dsSceneInstanceData_bindInstance(instanceData->instanceData[i], index, values))
			return false;
	}

	return true;
}
