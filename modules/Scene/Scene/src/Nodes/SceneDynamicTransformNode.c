/*
 * Copyright 2026 Aaron Barany
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

#include <DeepSea/Scene/Nodes/SceneDynamicTransformNode.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>

#include <DeepSea/Math/RigidTransform3.h>
#include <DeepSea/Math/Vector3x.h>

#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/Nodes/SceneTreeNode.h>

static void dsSceneDynamicTransformNode_setupTreeNode(dsSceneNode* node, dsSceneTreeNode* treeNode)
{
	treeNode->baseStepTransform = &((dsSceneDynamicTransformNode*)node)->transform;
}

static void dsSceneDynamicTransformNode_shift(dsSceneNode* node, const dsVector3xf* shift)
{
	dsSceneDynamicTransformNode* transformNode = (dsSceneDynamicTransformNode*)node;
	dsVector3xf_add(
		&transformNode->transform.position, &transformNode->transform.position, shift);
}

static void dsSceneDynamicTransformNode_destroy(dsSceneNode* node)
{
	DS_VERIFY(dsAllocator_free(node->allocator, node));
}

const char* const dsSceneDynamicTransformNode_typeName = "DynamicTransformNode";

static dsSceneNodeType nodeType =
{
	.setupTreeNodeFunc = &dsSceneDynamicTransformNode_setupTreeNode,
	.shiftNodeFunc = &dsSceneDynamicTransformNode_shift,
	.destroyFunc = &dsSceneDynamicTransformNode_destroy
};

const dsSceneNodeType* dsSceneDynamicTransformNode_type(void)
{
	return &nodeType;
}

dsSceneDynamicTransformNode* dsSceneDynamicTransformNode_create(dsAllocator* allocator,
	const dsRigidTransform3f* transform, const char* const* itemLists, uint32_t itemListCount)
{
	if (!allocator || (!itemLists && itemListCount > 0))
	{
		errno = EINVAL;
		return NULL;
	}

	size_t fullSize = sizeof(dsSceneDynamicTransformNode);
	if (itemListCount > 0)
	{
		size_t itemListSize = dsSceneNode_itemListsAllocSize(itemLists, itemListCount);
		if (itemListSize == 0 || !dsAddAlignedSize(&fullSize, itemListSize, DS_ALLOC_ALIGNMENT))
			return NULL;
	}

	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsSceneDynamicTransformNode* transformNode = DS_ALLOCATE_OBJECT(
		&bufferAlloc, dsSceneDynamicTransformNode);
	DS_ASSERT(transformNode);

	const char* const* itemListsCopy = dsSceneNode_copyItemLists(
		(dsAllocator*)&bufferAlloc, itemLists, itemListCount);
	DS_ASSERT(itemListCount == 0 || itemListsCopy);

	dsSceneNode* baseNode = (dsSceneNode*)transformNode;
	if (!dsSceneNode_initialize(
			baseNode, allocator, dsSceneDynamicTransformNode_type(), itemListsCopy, itemListCount))
	{
		if (allocator->freeFunc)
			DS_VERIFY(dsAllocator_free(allocator, transformNode));
		return NULL;
	}

	if (transform)
		transformNode->transform = *transform;
	else
		dsRigidTransform3f_identity(&transformNode->transform);
	return transformNode;
}

bool dsSceneDynamicTransformNode_setTransform(
	dsSceneDynamicTransformNode* node, const dsRigidTransform3f* transform)
{
	if (!node || !transform)
	{
		errno = EINVAL;
		return false;
	}

	node->transform = *transform;

	dsSceneNode* baseNode = (dsSceneNode*)node;
	for (uint32_t i = 0; i < baseNode->treeNodeCount; ++i)
		dsSceneTreeNode_markDirty(baseNode->treeNodes[i]);
	return true;
}
