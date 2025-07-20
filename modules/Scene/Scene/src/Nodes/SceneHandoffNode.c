/*
 * Copyright 2025 Aaron Barany
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

#include <DeepSea/Scene/Nodes/SceneHandoffNode.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

static void dsSceneHandoffNode_destroy(dsSceneNode* node)
{
	DS_VERIFY(dsAllocator_free(node->allocator, node));
}

const char* const dsSceneHandoffNode_typeName = "HandoffNode";

static dsSceneNodeType nodeType =
{
	.destroyFunc = &dsSceneHandoffNode_destroy
};

const dsSceneNodeType* dsSceneHandoffNode_type(void)
{
	return &nodeType;
}

dsSceneHandoffNode* dsSceneHandoffNode_create(dsAllocator* allocator, float transitionTime,
	const char* const* itemLists, uint32_t itemListCount)
{
	if (!allocator || transitionTime <= 0.0f || (!itemLists && itemListCount > 0))
	{
		errno = EINVAL;
		return NULL;
	}

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSceneHandoffNode)) +
		dsSceneNode_itemListsAllocSize(itemLists, itemListCount);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsSceneHandoffNode* node = DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneHandoffNode);
	DS_ASSERT(node);

	const char* const* itemListsCopy = dsSceneNode_copyItemLists((dsAllocator*)&bufferAlloc,
		itemLists, itemListCount);
	DS_ASSERT(itemListCount == 0 || itemListsCopy);

	if (!dsSceneNode_initialize((dsSceneNode*)node, allocator, dsSceneHandoffNode_type(),
			itemListsCopy, itemListCount))
	{
		if (allocator->freeFunc)
			DS_VERIFY(dsAllocator_free(allocator, node));
		return NULL;
	}

	node->transitionTime = transitionTime;
	return node;
}
