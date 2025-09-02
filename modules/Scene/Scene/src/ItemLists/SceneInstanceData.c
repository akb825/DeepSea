/*
 * Copyright 2019-2025 Aaron Barany
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

#include <DeepSea/Scene/ItemLists/SceneInstanceData.h>

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Error.h>

bool dsSceneInstanceData_populateData(dsSceneInstanceData* instanceData,
	const dsView* view, dsCommandBuffer* commandBuffer, const dsSceneTreeNode* const* instances,
	uint32_t instanceCount)
{
	if (!instanceData || !instanceData->type || !instanceData->type->populateDataFunc || !view ||
		(instanceData->needsCommandBuffer && !commandBuffer) || (!instances && instanceCount > 0))
	{
		errno = EINVAL;
		return false;
	}

	return instanceData->type->populateDataFunc(instanceData, view, commandBuffer, instances,
		instanceCount);
}

bool dsSceneInstanceData_bindInstance(dsSceneInstanceData* instanceData, uint32_t index,
	dsSharedMaterialValues* values)
{
	if (!instanceData || !instanceData->type || !instanceData->type->bindInstanceFunc ||
		(!values && instanceData->valueCount > 0))
	{
		errno = EINVAL;
		return false;
	}

	return instanceData->type->bindInstanceFunc(instanceData, index, values);
}

bool dsSceneInstanceData_finish(dsSceneInstanceData* instanceData)
{
	if (!instanceData || !instanceData->type || !instanceData->type->finishFunc)
	{
		errno = EINVAL;
		return false;
	}

	return instanceData->type->finishFunc(instanceData);
}

uint32_t dsSceneInstanceData_hash(const dsSceneInstanceData* instanceData, uint32_t seed)
{
	if (!instanceData || !instanceData->type)
		return 0;

	uint32_t hash = dsHashCombinePointer(seed, instanceData->type);
	uint32_t hashValues[2] = {instanceData->valueCount, instanceData->needsCommandBuffer};
	hash = dsHashCombineBytes(hash, hashValues, sizeof(hashValues));

	dsHashSceneInstanceDataFunction hashFunc = instanceData->type->hashFunc;
	if (hashFunc)
		hash = hashFunc(instanceData, hash);
	return hash;
}

bool dsSceneInstanceData_equal(const dsSceneInstanceData* left, const dsSceneInstanceData* right)
{
	if (left == right)
		return true;
	else if (!left || !right || !left->type || left->type != right->type ||
		left->valueCount != right->valueCount ||
		left->needsCommandBuffer != right->needsCommandBuffer)
	{
		return false;
	}

	dsSceneInstanceDataEqualFunction equalFunc = left->type->equalFunc;
	if (equalFunc)
		return equalFunc(left, right);
	return true;
}

bool dsSceneInstanceData_destroy(dsSceneInstanceData* instanceData)
{
	if (!instanceData || !instanceData->type || !instanceData->type->destroyFunc)
		return true;

	return instanceData->type->destroyFunc(instanceData);
}
