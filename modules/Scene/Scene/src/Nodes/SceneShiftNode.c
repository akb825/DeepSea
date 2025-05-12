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

#include <DeepSea/Scene/Nodes/SceneShiftNode.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

#include <DeepSea/Math/Vector3.h>

#include <DeepSea/Scene/Nodes/SceneNode.h>

#include <string.h>

static void dsSceneShiftNode_destroy(dsSceneNode* node)
{
	DS_VERIFY(dsAllocator_free(node->allocator, node));
}

const char* const dsSceneShiftNode_typeName = "ShiftNode";

static dsSceneNodeType nodeType;
const dsSceneNodeType* dsSceneShiftNode_type(void)
{
	return &nodeType;
}

dsSceneShiftNode* dsSceneShiftNode_create(dsAllocator* allocator,
	const dsVector3d* origin, const char* const* itemLists, uint32_t itemListCount)
{
	if (!allocator || (!itemLists && itemListCount > 0))
	{
		errno = EINVAL;
		return NULL;
	}

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSceneShiftNode)) +
		dsSceneNode_itemListsAllocSize(itemLists, itemListCount);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsSceneShiftNode* node = DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneShiftNode);
	DS_ASSERT(node);

	const char* const* itemListsCopy = dsSceneNode_copyItemLists((dsAllocator*)&bufferAlloc,
		itemLists, itemListCount);
	DS_ASSERT(itemListCount == 0 || itemListsCopy);

	if (!dsSceneNode_initialize((dsSceneNode*)node, allocator, dsSceneShiftNode_type(),
			itemListsCopy, itemListCount, &dsSceneShiftNode_destroy))
	{
		if (allocator->freeFunc)
			DS_VERIFY(dsAllocator_free(allocator, node));
		return NULL;
	}

	if (origin)
		node->origin = *origin;
	else
		memset(&node->origin, 0, sizeof(dsVector3d));

	return node;
}

bool dsSceneShiftNode_setOrigin(dsSceneShiftNode* node, const dsVector3d* origin)
{
	if (!node || !origin)
	{
		errno = EINVAL;
		return false;
	}

	if (dsVector3_equal(node->origin, *origin))
		return true;

	dsVector3d offset;
	dsVector3_sub(offset, node->origin, *origin);
	dsVector3f offset3f;
	dsConvertDoubleToFloat(offset3f, offset);

	dsSceneNode* baseNode = (dsSceneNode*)node;
	for (uint32_t i = 0; i < baseNode->childCount; ++i)
	{
		dsSceneNode* child = baseNode->children[i];
		if (child->shiftNodeFunc)
			child->shiftNodeFunc(child, &offset3f);
	}

	return true;
}

bool dsSceneShiftNode_getChildPosition(dsVector3d* outPosition, dsSceneTreeNode* node)
{
	if (!outPosition || !node)
	{
		errno = EINVAL;
		return false;
	}

	dsConvertFloatToDouble(*outPosition, *(dsVector3f*)(node->transform.columns + 3));
	do
	{
		if (dsSceneNode_isOfType(node->node, dsSceneShiftNode_type()))
		{
			dsSceneShiftNode* shiftNode = (dsSceneShiftNode*)node->node;
			dsVector3_add(*outPosition, shiftNode->origin, *outPosition);
			break;
		}

		node = node->parent;
	} while (node);
	return true;
}

bool dsSceneShiftNode_getChildTransform(dsMatrix44d* outTransform, dsSceneTreeNode* node)
{
	if (!outTransform || !node)
	{
		errno = EINVAL;
		return false;
	}

	dsConvertFloatToDouble(*outTransform, node->transform);
	do
	{
		if (dsSceneNode_isOfType(node->node, dsSceneShiftNode_type()))
		{
			dsSceneShiftNode* shiftNode = (dsSceneShiftNode*)node->node;
			dsVector3_add(outTransform->columns[3], shiftNode->origin, outTransform->columns[3]);
			break;
		}

		node = node->parent;
	} while (node);
	return true;
}
