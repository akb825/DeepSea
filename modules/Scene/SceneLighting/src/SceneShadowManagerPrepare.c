/*
 * Copyright 2021-2025 Aaron Barany
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

#include <DeepSea/SceneLighting/SceneShadowManagerPrepare.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/UniqueNameID.h>

#include <DeepSea/SceneLighting/SceneLightShadows.h>
#include <DeepSea/SceneLighting/SceneShadowManager.h>

#include <string.h>

typedef struct dsSceneShadowManagerPrepare
{
	dsSceneItemList itemList;
	dsSceneShadowManager* shadowManager;
} dsSceneShadowManagerPrepare;

static void dsSceneShadowManagerPrepare_commit(dsSceneItemList* itemList,
	const dsView* view, dsCommandBuffer* commandBuffer)
{
	DS_UNUSED(commandBuffer);
	dsSceneShadowManagerPrepare* prepare = (dsSceneShadowManagerPrepare*)itemList;
	dsSceneShadowManager_prepare(prepare->shadowManager, view, itemList);
}

static void dsSceneShadowManagerPrepare_destroy(dsSceneItemList* itemList)
{
	if (itemList->allocator)
		DS_VERIFY(dsAllocator_free(itemList->allocator, itemList));
}

const char* const dsSceneShadowManagerPrepare_typeName = "ShadowManagerPrepare";

dsSceneItemListType dsSceneShadowManagerPrepare_type(void)
{
	static int type;
	return &type;
}

dsSceneItemList* dsSceneShadowManagerPrepare_create(dsAllocator* allocator, const char* name,
	dsSceneShadowManager* shadowManager)
{
	if (!allocator || !shadowManager)
	{
		errno = EINVAL;
		return NULL;
	}

	size_t nameLen = strlen(name) + 1;
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSceneShadowManagerPrepare)) +
		DS_ALIGNED_SIZE(nameLen);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsSceneShadowManagerPrepare* prepare =
		DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneShadowManagerPrepare);
	DS_ASSERT(prepare);

	dsSceneItemList* itemList = (dsSceneItemList*)prepare;
	itemList->allocator = dsAllocator_keepPointer(allocator);
	itemList->type = dsSceneShadowManagerPrepare_type();
	itemList->name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
	DS_ASSERT(itemList->name);
	memcpy((void*)itemList->name, name, nameLen);
	itemList->nameID = dsUniqueNameID_create(name);
	itemList->globalValueCount = dsSceneShadowManager_globalTransformGroupCount(shadowManager);
	itemList->needsCommandBuffer = false;
	itemList->addNodeFunc = NULL;
	itemList->updateNodeFunc = NULL;
	itemList->removeNodeFunc = NULL;
	itemList->reparentNodeFunc = NULL;
	itemList->preTransformUpdateFunc = NULL;
	itemList->updateFunc = NULL;
	itemList->preRenderPassFunc = NULL;
	itemList->commitFunc = &dsSceneShadowManagerPrepare_commit;
	itemList->destroyFunc = &dsSceneShadowManagerPrepare_destroy;

	prepare->shadowManager = shadowManager;

	return itemList;
}
