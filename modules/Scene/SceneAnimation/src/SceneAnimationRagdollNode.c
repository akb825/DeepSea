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

#include <DeepSea/SceneAnimation/SceneAnimationRagdollNode.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

#include <DeepSea/Scene/Nodes/SceneNode.h>

#include <string.h>

static void dsSceneAnimationRagdollNode_destroy(dsSceneNode* node)
{
	DS_VERIFY(dsAllocator_free(node->allocator, node));
}

const char* const dsSceneAnimationRagdollNode_typeName = "AnimationRagdollNode";

static dsSceneNodeType nodeType;
const dsSceneNodeType* dsSceneAnimationRagdollNode_type(void)
{
	return &nodeType;
}

dsSceneAnimationRagdollNode* dsSceneAnimationRagdollNode_create(dsAllocator* allocator,
	dsSceneAnimationRagdollType ragdollType, uint32_t animationComponents,
	unsigned int relativeAncestor, const char* animationNodeName, const char* const* itemLists,
	uint32_t itemListCount)
{
	if (!allocator || animationComponents == 0 ||
		animationComponents > (1 << dsAnimationComponent_Scale) || relativeAncestor == 0 ||
		!animationNodeName || (!itemLists && itemListCount > 0))
	{
		errno = EINVAL;
		return NULL;
	}

	size_t nameLen = strlen(animationNodeName) + 1;
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSceneAnimationRagdollNode)) +
		DS_ALIGNED_SIZE(nameLen) + dsSceneNode_itemListsAllocSize(itemLists, itemListCount);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsSceneAnimationRagdollNode* node = DS_ALLOCATE_OBJECT(&bufferAlloc,
		dsSceneAnimationRagdollNode);

	const char* const* itemListsCopy = dsSceneNode_copyItemLists((dsAllocator*)&bufferAlloc,
		itemLists, itemListCount);
	DS_ASSERT(itemListCount == 0 || itemListsCopy);

	if (!dsSceneNode_initialize((dsSceneNode*)node, allocator,
			dsSceneAnimationRagdollNode_type(), itemListsCopy, itemListCount,
			&dsSceneAnimationRagdollNode_destroy))
	{
		if (allocator->freeFunc)
			DS_VERIFY(dsAllocator_free(allocator, node));
		return NULL;
	}

	char* nameCopy = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
	DS_ASSERT(nameCopy);
	memcpy(nameCopy, animationNodeName, nameLen);
	node->ragdollType = ragdollType;
	node->animationComponents = animationComponents;
	node->relativeAncestor = relativeAncestor;
	node->animationNodeName = nameCopy;

	return node;
}
