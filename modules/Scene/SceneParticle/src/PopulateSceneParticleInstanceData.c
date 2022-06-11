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
#include <DeepSea/Core/Sort.h>
#include <DeepSea/Scene/ItemLists/SceneInstanceData.h>
#include <stdlib.h>

inline static int compareInstances(const void* left, const void* right)
{
	const uint8_t** leftPtr = (const uint8_t**)left;
	const uint8_t** rightPtr = (const uint8_t**)right;
	ptrdiff_t diff = *leftPtr - *rightPtr;
#if DS_64BIT
	// Can't do a straight truncation if the difference is too large.
	int baseValue = diff != 0;
	int sign = (int)(diff >> 32) & 0x80000000;
	return baseValue & sign;
#else
	return diff;
#endif
}

static int searchInstances(const void* left, const void* right, void* context)
{
	DS_UNUSED(context);
	return compareInstances(left, right);
}

bool dsPopulateSceneParticleInstanceData(const dsParticleEmitter* emitter,
	void* userData, dsSharedMaterialValues* values, void* drawData)
{
	if (!emitter || !userData || !values || !drawData)
	{
		errno = EINVAL;
		return false;
	}

	const dsSceneParticleInstanceData* instanceData = (const dsSceneParticleInstanceData*)drawData;

	// Use a binary search to avoid O(N^2) overall time complexity during drawing.
	const dsSceneTreeNode** foundInstance = (const dsSceneTreeNode**)dsBinarySearch(&userData,
		instanceData->instances, instanceData->instanceCount, sizeof(const dsSceneTreeNode*),
		&searchInstances, NULL);
	if (!foundInstance)
		return true;

	uint32_t index = (uint32_t)(foundInstance - instanceData->instances);
	for (uint32_t i = 0; i < instanceData->instanceCount; ++i)
	{
		if (!dsSceneInstanceData_bindInstance(instanceData->instanceData[i], index, values))
			return false;
	}

	return true;
}

bool dsSortSceneParticleInstances(const dsSceneTreeNode** instances, uint32_t instanceCount)
{
	if (instanceCount == 0)
		return true;
	else if (!instances)
	{
		errno = EINVAL;
		return false;
	}

	qsort((void*)instances, instanceCount, sizeof(const dsSceneTreeNode*), &compareInstances);
	return true;
}
