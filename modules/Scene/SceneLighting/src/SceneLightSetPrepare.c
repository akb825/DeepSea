/*
 * Copyright 2020-2022 Aaron Barany
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
#include <DeepSea/SceneLighting/SceneLightSet.h>

#include <string.h>

struct dsSceneLightSetPrepare
{
	dsSceneItemList itemList;
	dsSceneLightSet* lightSet;
	float intensityThreshold;
};

static void dsSceneLightSetPrepare_commit(dsSceneItemList* itemList, const dsView* view,
	dsCommandBuffer* commandBuffer)
{
	DS_UNUSED(view);
	DS_UNUSED(commandBuffer);
	dsSceneLightSetPrepare* prepare = (dsSceneLightSetPrepare*)itemList;
	dsSceneLightSet_prepare(prepare->lightSet, prepare->intensityThreshold);
}

const char* const dsSceneLightSetPrepare_typeName = "LightSetPrepare";

dsSceneItemListType dsSceneLightSetPrepare_type(void)
{
	static int type;
	return &type;
}

dsSceneLightSetPrepare* dsSceneLightSetPrepare_create(dsAllocator* allocator, const char* name,
	dsSceneLightSet* lightSet, float intensityThreshold)
{
	if (!allocator || !name || !lightSet || intensityThreshold <= 0)
	{
		errno = EINVAL;
		return NULL;
	}

	size_t nameLen = strlen(name) + 1;
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSceneLightSetPrepare)) + DS_ALIGNED_SIZE(nameLen);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsSceneLightSetPrepare* prepare = DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneLightSetPrepare);
	DS_ASSERT(prepare);

	dsSceneItemList* itemList = (dsSceneItemList*)prepare;
	itemList->allocator = dsAllocator_keepPointer(allocator);
	itemList->type = dsSceneLightSetPrepare_type();
	itemList->name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
	DS_ASSERT(itemList->name);
	memcpy((void*)itemList->name, name, nameLen);
	itemList->nameID = dsHashString(name);
	itemList->globalValueCount = 0;
	itemList->needsCommandBuffer = false;
	itemList->addNodeFunc = NULL;
	itemList->updateNodeFunc = NULL;
	itemList->removeNodeFunc = NULL;
	itemList->updateFunc = NULL;
	itemList->commitFunc = &dsSceneLightSetPrepare_commit;
	itemList->destroyFunc = (dsDestroySceneItemListFunction)&dsSceneLightSetPrepare_destroy;

	prepare->lightSet = lightSet;
	prepare->intensityThreshold = intensityThreshold;

	return prepare;
}

const dsSceneLightSet* dsSceneLightSetPrepare_getLightSet(const dsSceneLightSetPrepare* prepare)
{
	if (!prepare)
	{
		errno = EINVAL;
		return NULL;
	}

	return prepare->lightSet;
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
