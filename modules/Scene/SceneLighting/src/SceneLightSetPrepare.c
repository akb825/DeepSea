/*
 * Copyright 2020-2021 Aaron Barany
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

#include <DeepSea/SceneLighting/SceneLightSetPrepare.h>

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Scene/SceneGlobalData.h>
#include <DeepSea/SceneLighting/SceneLightSet.h>

#include <string.h>

struct dsSceneLightSetPrepare
{
	dsSceneGlobalData globalData;
	dsSceneLightSet** lightSets;
	uint32_t lightSetCount;
	float intensityThreshold;
};

const char* const dsSceneLightSetPrepare_typeName = "LightSetPrepare";

bool dsSceneLightSetPrepare_populateData(dsSceneGlobalData* globalData, const dsView* view,
	dsCommandBuffer* commandBuffer)
{
	DS_UNUSED(view);
	DS_UNUSED(commandBuffer);
	dsSceneLightSetPrepare* prepare = (dsSceneLightSetPrepare*)globalData;
	for (uint32_t i = 0; i < prepare->lightSetCount; ++i)
		dsSceneLightSet_prepare(prepare->lightSets[i], prepare->intensityThreshold);
	return true;
}

bool dsSceneLightSetPrepare_destroyGlobalData(dsSceneGlobalData* globalData)
{
	dsSceneLightSetPrepare_destroy((dsSceneLightSetPrepare*)globalData);
	return true;
}

dsSceneLightSetPrepare* dsSceneLightSetPrepare_create(dsAllocator* allocator,
	dsSceneLightSet* const* lightSets, uint32_t lightSetCount, float intensityThreshold)
{
	if (!allocator || !lightSets || lightSetCount == 0 || intensityThreshold <= 0)
	{
		errno = EINVAL;
		return NULL;
	}

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSceneLightSetPrepare)) +
		DS_ALIGNED_SIZE(sizeof(dsSceneLightSet*)*lightSetCount);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsSceneLightSetPrepare* prepare = DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneLightSetPrepare);
	DS_ASSERT(prepare);

	dsSceneGlobalData* globalData = (dsSceneGlobalData*)prepare;
	globalData->allocator = dsAllocator_keepPointer(allocator);
	globalData->valueCount = 0;
	globalData->populateDataFunc = &dsSceneLightSetPrepare_populateData;
	globalData->finishFunc = NULL;
	globalData->destroyFunc = dsSceneLightSetPrepare_destroyGlobalData;

	prepare->lightSets = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsSceneLightSet*, lightSetCount);
	DS_ASSERT(prepare->lightSets);
	memcpy(prepare->lightSets, lightSets, sizeof(dsSceneLightSet*)*lightSetCount);
	prepare->lightSetCount = lightSetCount;
	prepare->intensityThreshold = intensityThreshold;

	return prepare;
}

float dsSceneLightSetPrepare_getIntensityThreshold(const dsSceneLightSetPrepare* prepare)
{
	if (!prepare)
	{
		errno = EINVAL;
		return 0;
	}

	return prepare->intensityThreshold;
}

bool dsSceneLightSetPrepare_setIntensityThreshold(dsSceneLightSetPrepare* prepare,
	float intensityThreshold)
{
	if (!prepare || intensityThreshold <= 0)
	{
		errno = EINVAL;
		return false;
	}

	prepare->intensityThreshold = intensityThreshold;
	return true;
}

void dsSceneLightSetPrepare_destroy(dsSceneLightSetPrepare* prepare)
{
	if (!prepare)
		return;

	dsSceneItemList* itemList = (dsSceneItemList*)prepare;
	if (itemList->allocator)
		DS_VERIFY(dsAllocator_free(itemList->allocator, itemList));
}
