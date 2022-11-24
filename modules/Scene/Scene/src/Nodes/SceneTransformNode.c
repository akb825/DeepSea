/*
 * Copyright 2019-2022 Aaron Barany
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

#include <DeepSea/Scene/Nodes/SceneTransformNode.h>

#include "Nodes/SceneTreeNodeInternal.h"
#include "SceneTypes.h"

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>

#include <DeepSea/Math/Matrix44.h>

#include <DeepSea/Scene/Nodes/SceneNode.h>

static void dsSceneTransformnode_destroy(dsSceneNode* node)
{
	DS_VERIFY(dsAllocator_free(node->allocator, node));
}

const char* const dsSceneTransformNode_typeName = "TransformNode";

static dsSceneNodeType nodeType;
const dsSceneNodeType* dsSceneTransformNode_type(void)
{
	return &nodeType;
}

dsSceneTransformNode* dsSceneTransformNode_create(dsAllocator* allocator,
	const dsMatrix44f* transform)
{
	if (!allocator)
	{
		errno = EINVAL;
		return NULL;
	}

	dsSceneTransformNode* node = DS_ALLOCATE_OBJECT(allocator, dsSceneTransformNode);
	if (!node)
		return NULL;

	if (!dsSceneNode_initialize((dsSceneNode*)node, allocator, dsSceneTransformNode_type(), NULL, 0,
			&dsSceneTransformnode_destroy))
	{
		if (allocator->freeFunc)
			DS_VERIFY(dsAllocator_free(allocator, node));
		return NULL;
	}

	if (transform)
		node->transform = *transform;
	else
		dsMatrix44_identity(node->transform);
	return node;
}

bool dsSceneTransformNode_setTransform(dsSceneTransformNode* node, const dsMatrix44f* transform)
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
