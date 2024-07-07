/*
 * Copyright 2023-2024 Aaron Barany
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

#include <DeepSea/SceneAnimation/SceneAnimationTransformNode.h>

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

#include <DeepSea/Math/Matrix44.h>

#include <DeepSea/Scene/Nodes/SceneTransformNode.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>

#include <string.h>

static void dsSceneAnimationTransformNode_destroy(dsSceneNode* node)
{
	DS_VERIFY(dsAllocator_free(node->allocator, node));
}

const char* const dsSceneAnimationTransformNode_typeName = "AnimationTransformNode";

static dsSceneNodeType nodeType;
const dsSceneNodeType* dsSceneAnimationTransformNode_type(void)
{
	return &nodeType;
}

const dsSceneNodeType* dsSceneAnimationTransformNode_setupParentType(dsSceneNodeType* type)
{
	dsSceneNode_setupParentType(&nodeType, dsSceneTransformNode_type());
	return dsSceneNode_setupParentType(type, &nodeType);
}

dsSceneAnimationTransformNode* dsSceneAnimationTransformNode_create(dsAllocator* allocator,
	const char* animationNodeName, const char* const* itemLists, uint32_t itemListCount)
{
	if (!allocator || !animationNodeName || (!itemLists && itemListCount > 0))
	{
		errno = EINVAL;
		return NULL;
	}

	size_t nameLen = strlen(animationNodeName) + 1;
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSceneAnimationTransformNode)) +
		DS_ALIGNED_SIZE(nameLen) + dsSceneNode_itemListsAllocSize(itemLists, itemListCount);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsSceneAnimationTransformNode* node = DS_ALLOCATE_OBJECT(&bufferAlloc,
		dsSceneAnimationTransformNode);

	const char* const* itemListsCopy = dsSceneNode_copyItemLists((dsAllocator*)&bufferAlloc,
		itemLists, itemListCount);
	DS_ASSERT(itemListCount == 0 || itemListsCopy);

	if (!dsSceneNode_initialize((dsSceneNode*)node, allocator,
			dsSceneAnimationTransformNode_setupParentType(NULL), itemListsCopy, itemListCount,
			&dsSceneAnimationTransformNode_destroy))
	{
		if (allocator->freeFunc)
			DS_VERIFY(dsAllocator_free(allocator, node));
		return NULL;
	}

	dsMatrix44_identity(((dsSceneTransformNode*)node)->transform);

	char* nameCopy = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
	DS_ASSERT(nameCopy);
	memcpy(nameCopy, animationNodeName, nameLen);
	node->animationNodeName = nameCopy;
	node->animationNodeID = dsHashString(animationNodeName);

	return node;
}
