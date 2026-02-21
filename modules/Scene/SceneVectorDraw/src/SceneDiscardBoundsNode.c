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

#include <DeepSea/SceneVectorDraw/SceneDiscardBoundsNode.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

#include <DeepSea/Geometry/AlignedBox2.h>

#include <DeepSea/Scene/Nodes/SceneNode.h>

static void dsSceneDiscardBoundsNode_destroy(dsSceneNode* node)
{
	DS_VERIFY(dsAllocator_free(node->allocator, node));
}

const char* const dsSceneDiscardBoundsNode_typeName = "DiscardBoundsNode";

static dsSceneNodeType nodeType =
{
	.destroyFunc = &dsSceneDiscardBoundsNode_destroy
};

const dsSceneNodeType* dsSceneDiscardBoundsNode_type(void)
{
	return &nodeType;
}

dsSceneDiscardBoundsNode* dsSceneDiscardBoundsNode_create(
	dsAllocator* allocator, const dsAlignedBox2f* bounds)
{
	if (!allocator)
	{
		errno = EINVAL;
		return NULL;
	}

	dsSceneDiscardBoundsNode* node = DS_ALLOCATE_OBJECT(allocator, dsSceneDiscardBoundsNode);
	if (!node)
		return NULL;

	dsSceneNode* baseNode = (dsSceneNode*)node;
	if (!dsSceneNode_initialize(baseNode, allocator, dsSceneDiscardBoundsNode_type(), NULL, 0))
	{
		if (allocator->freeFunc)
			DS_VERIFY(dsAllocator_free(allocator, node));
		return NULL;
	}

	if (bounds)
		node->discardBounds = *bounds;
	else
		dsAlignedBox2f_makeInvalid(&node->discardBounds);
	return node;
}

const dsAlignedBox2f* dsSceneDiscardBoundsNode_getDiscardBoundsForInstance(
	dsMatrix44f* outTransform, const dsSceneTreeNode* treeNode)
{
	while (treeNode && !dsSceneNode_isOfType(treeNode->node, dsSceneDiscardBoundsNode_type()))
		treeNode = treeNode->parent;
	if (!treeNode || !outTransform)
		return NULL;

	const dsSceneDiscardBoundsNode* discardBoundsNode =
		(const dsSceneDiscardBoundsNode*)treeNode->node;
	if (!dsAlignedBox2_isValid(discardBoundsNode->discardBounds))
		return NULL;

	*outTransform = treeNode->transform;
	return &discardBoundsNode->discardBounds;
}
