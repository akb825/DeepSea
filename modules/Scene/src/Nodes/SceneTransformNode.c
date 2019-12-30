/*
 * Copyright 2019 Aaron Barany
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

#include "Nodes/SceneTreeNode.h"
#include "SceneTypes.h"
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>

static dsSceneNodeType nodeType;

static void destroy(dsSceneNode* node)
{
	DS_VERIFY(dsAllocator_free(node->allocator, node));
}

const char* const dsSceneTransformNode_typeName = "TransformNode";

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
			&destroy))
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
	{
		dsSceneTreeNode* treeNode = baseNode->treeNodes[i];
		dsScene* scene = dsSceneTreeNode_getScene(treeNode);
		DS_ASSERT(scene);

		treeNode->dirty = true;
		// Since the dirty flag is used, don't bother a linear search to see if already on the list.
		uint32_t index = scene->dirtyNodeCount;
		if (!DS_RESIZEABLE_ARRAY_ADD(scene->allocator, scene->dirtyNodes, scene->dirtyNodeCount,
				scene->maxDirtyNodes, 1))
		{
			continue;
		}

		scene->dirtyNodes[index] = treeNode;
	}
	return true;
}
