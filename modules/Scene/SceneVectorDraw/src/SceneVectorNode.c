/*
 * Copyright 2020-2025 Aaron Barany
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

#include <DeepSea/SceneVectorDraw/SceneVectorNode.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Render/Resources/Material.h>

#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/SceneResources.h>

#include <string.h>

static dsSceneNodeType nodeType =
{
	.destroyFunc = dsSceneVectorNode_destroy
};

const dsSceneNodeType* dsSceneVectorNode_type(void)
{
	return &nodeType;
}

dsSceneVectorNode* dsSceneVectorNode_create(dsAllocator* allocator, size_t structSize, int32_t z,
	const char* const* itemLists, uint32_t itemListCount, dsSceneResources** resources,
	uint32_t resourceCount)
{
	if (!allocator || structSize < sizeof(dsSceneVectorImageNode) ||
		(!itemLists && itemListCount > 0) || (!resources && resourceCount > 0))
	{
		errno = EINVAL;
		return NULL;
	}

	for (uint32_t i = 0; i < resourceCount; ++i)
	{
		if (!resources[i])
		{
			errno = EINVAL;
			return NULL;
		}
	}

	size_t itemListsSize = dsSceneNode_itemListsAllocSize(itemLists, itemListCount);
	if (itemListsSize == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	size_t fullSize = DS_ALIGNED_SIZE(structSize) + itemListsSize +
		DS_ALIGNED_SIZE(sizeof(dsSceneResources*)*resourceCount);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsSceneVectorNode* node =
		(dsSceneVectorNode*)dsAllocator_alloc((dsAllocator*)&bufferAlloc, structSize);
	DS_ASSERT(node);

	const char* const* itemListsCopy = dsSceneNode_copyItemLists((dsAllocator*)&bufferAlloc,
		itemLists, itemListCount);
	DS_ASSERT(itemListCount == 0 || itemListsCopy);

	if (!dsSceneNode_initialize((dsSceneNode*)node, allocator, dsSceneVectorNode_type(),
			itemListsCopy, itemListCount))
	{
		if (allocator->freeFunc)
			DS_VERIFY(dsAllocator_free(allocator, node));
		return NULL;
	}

	if (resourceCount > 0)
	{
		node->resources = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsSceneResources*, resourceCount);
		DS_ASSERT(node->resources);
		for (uint32_t i = 0; i < resourceCount; ++i)
			node->resources[i] = dsSceneResources_addRef(resources[i]);
		node->resourceCount = resourceCount;
	}
	else
	{
		node->resources = NULL;
		node->resourceCount = 0;
	}

	node->z = z;

	return node;
}

void dsSceneVectorNode_destroy(dsSceneNode* node)
{
	DS_ASSERT(dsSceneNode_isOfType(node, dsSceneVectorNode_type()));
	dsSceneVectorNode* vectorNode = (dsSceneVectorNode*)node;
	for (uint32_t i = 0; i < vectorNode->resourceCount; ++i)
		dsSceneResources_freeRef(vectorNode->resources[i]);
	DS_VERIFY(dsAllocator_free(node->allocator, node));
}
