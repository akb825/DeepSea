/*
 * Copyright 2025-2026 Aaron Barany
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
#include <DeepSea/Math/Vector3x.h>
#include <DeepSea/Math/Vector4.h>

#include <DeepSea/Scene/Nodes/SceneNode.h>

#include <string.h>

static void offsetPositionsRec(dsSceneTreeNode* node, const dsVector3xf* offset)
{
	// Update both previous and current transforms, as this represents a shift in reference for the
	// same final world position. Node world transforms are always updated
	dsVector4f_add(node->prevFrameWorldTransform.columns + 3,
		node->prevFrameWorldTransform.columns + 3, offset);
	dsVector4f_add(node->curFrameWorldTransform.columns + 3,
		node->curFrameWorldTransform.columns + 3, offset);

	// Also update the local step transforms when the parent is ignored.
	if (node->noParentTransform)
	{
		dsVector3xf_add(&node->prevStepLocalTransform.position,
			&node->prevStepLocalTransform.position, offset);
		dsVector3xf_add(&node->curStepLocalTransform.position,
			&node->curStepLocalTransform.position, offset);
	}

	for (uint32_t i = 0; i < node->childCount; ++i)
		offsetPositionsRec(node->children[i], offset);
}

static void offsetBasePositions(dsSceneTreeNode* node, const dsVector3xf* offset)
{
	// Update both previous and current transforms, as this represents a shift in reference for the
	// same final world position.
	dsVector4f_add(node->prevFrameWorldTransform.columns + 3,
		node->prevFrameWorldTransform.columns + 3, offset);
	dsVector4f_add(node->curFrameWorldTransform.columns + 3,
		node->curFrameWorldTransform.columns + 3, offset);

	// Also update the local step transforms for the nodes directly under the shift node.
	dsVector3xf_add(&node->prevStepLocalTransform.position,
		&node->prevStepLocalTransform.position, offset);
	dsVector3xf_add(&node->curStepLocalTransform.position,
		&node->curStepLocalTransform.position, offset);

	for (uint32_t i = 0; i < node->childCount; ++i)
		offsetPositionsRec(node->children[i], offset);
}

static void dsSceneShiftNode_destroy(dsSceneNode* node)
{
	DS_VERIFY(dsAllocator_free(node->allocator, node));
}

const char* const dsSceneShiftNode_typeName = "ShiftNode";

static dsSceneNodeType nodeType =
{
	.destroyFunc = dsSceneShiftNode_destroy
};

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

	if (!dsSceneNode_initialize(
			(dsSceneNode*)node, allocator, dsSceneShiftNode_type(), itemListsCopy, itemListCount))
	{
		if (allocator->freeFunc)
			DS_VERIFY(dsAllocator_free(allocator, node));
		return NULL;
	}

	if (origin)
		node->origin = *origin;
	else
		node->origin.x = node->origin.y = node->origin.z = 0.0;

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
	dsVector3xf offset3f = {{(float)offset.x, (float)offset.y, (float)offset.z, 0.0f}};

	dsSceneNode* baseNode = (dsSceneNode*)node;
	for (uint32_t i = 0; i < baseNode->childCount; ++i)
	{
		dsSceneNode* child = baseNode->children[i];
		dsShiftSceneNodeFunction shiftNodeFunc = child->type->shiftNodeFunc;
		if (shiftNodeFunc)
			shiftNodeFunc(child, &offset3f);
		for (uint32_t j = 0; j < child->treeNodeCount; ++j)
			offsetBasePositions(child->treeNodes[j], &offset3f);
	}

	return true;
}

bool dsSceneShiftNode_getOriginForNode(dsVector3d* outOrigin, const dsSceneTreeNode* node)
{
	if (!outOrigin || !node)
	{
		errno = EINVAL;
		return false;
	}

	do
	{
		if (dsSceneNode_isOfType(node->node, dsSceneShiftNode_type()))
		{
			dsSceneShiftNode* shiftNode = (dsSceneShiftNode*)node->node;
			*outOrigin = shiftNode->origin;
			return true;
		}

		node = node->parent;
	} while (node);

	outOrigin->x = outOrigin->y = outOrigin->z = 0.0;
	return true;
}
