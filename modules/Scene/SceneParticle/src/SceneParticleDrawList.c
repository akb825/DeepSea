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

#include <DeepSea/SceneParticle/SceneParticleDrawList.h>

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Particle/ParticleDraw.h>

#include <string.h>

typedef struct dsSceneParticleDrawList
{
	dsSceneItemList itemList;

	dsParticleDraw** particleDraws;
	uint32_t particleDrawCount;
} dsSceneParticleDrawList;

static void dsSceneParticleDrawList_commit(dsSceneItemList* itemList, const dsView* view,
	dsCommandBuffer* commandBuffer)
{
	DS_PROFILE_DYNAMIC_SCOPE_START(itemList->name);

	dsSceneParticleDrawList* drawList = (dsSceneParticleDrawList*)itemList;
	for (uint32_t i = 0; i < drawList->particleDrawCount; ++i)
	{
		DS_CHECK(DS_SCENE_PARTICLE_LOG_TAG, dsParticleDraw_draw(drawList->particleDraws[i],
			commandBuffer, view->globalValues, &view->viewMatrix, &view->viewFrustum));
	}

	DS_PROFILE_SCOPE_END();
}

void dsSceneParticleDrawList_destroy(dsSceneItemList* itemList)
{
	DS_VERIFY(dsAllocator_free(itemList->allocator, itemList));
}

const char* const dsSceneParticleDrawList_typeName = "ParticleDrawList";

dsSceneItemListType dsSceneParticleDrawList_type(void)
{
	static int type;
	return &type;
}

dsSceneItemList* dsSceneParticleDrawList_create(dsAllocator* allocator, const char* name,
	dsParticleDraw* const* particleDraws, uint32_t particleDrawCount)
{
	if (!allocator || !name || !particleDraws || particleDrawCount == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	for (uint32_t i = 0; i < particleDrawCount; ++i)
	{
		if (!particleDraws[i])
		{
			errno = EINVAL;
			return false;
		}
	}

	size_t nameLen = strlen(name);
	size_t fullSize =
		DS_ALIGNED_SIZE(sizeof(dsSceneParticleDrawList)) + DS_ALIGNED_SIZE(nameLen + 1) +
		DS_ALIGNED_SIZE(sizeof(dsParticleDraw*)*particleDrawCount);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsSceneParticleDrawList* drawList =
		DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneParticleDrawList);
	DS_ASSERT(drawList);

	dsSceneItemList* itemList = (dsSceneItemList*)drawList;
	itemList->allocator = allocator;
	itemList->type = dsSceneParticleDrawList_type();
	itemList->name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen + 1);
	memcpy((void*)itemList->name, name, nameLen + 1);
	itemList->nameID = dsHashString(name);
	itemList->needsCommandBuffer = true;
	itemList->addNodeFunc = NULL;
	itemList->updateNodeFunc = NULL;
	itemList->removeNodeFunc = NULL;
	itemList->updateFunc = NULL;
	itemList->commitFunc = &dsSceneParticleDrawList_commit;
	itemList->destroyFunc = &dsSceneParticleDrawList_destroy;

	drawList->particleDraws = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsParticleDraw*,
		particleDrawCount);
	DS_ASSERT(drawList->particleDraws);
	drawList->particleDrawCount = particleDrawCount;

	return itemList;
}
