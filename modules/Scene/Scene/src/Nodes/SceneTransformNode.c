/*
 * Copyright 2019-2025 Aaron Barany
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

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>

#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Vector3.h>

#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/Nodes/SceneTreeNode.h>

static void dsSceneTransformNode_setupTreeNode(dsSceneNode* node, dsSceneTreeNode* treeNode)
{
	treeNode->baseTransform = &((dsSceneTransformNode*)node)->transform;
}

static void dsSceneTransformNode_shift(dsSceneNode* node, const dsVector3f* shift)
{
	dsSceneTransformNode* transformNode = (dsSceneTransformNode*)node;
	dsVector3_add(transformNode->transform.columns[3], transformNode->transform.columns[3], *shift);
	for (uint32_t i = 0; i < node->treeNodeCount; ++i)
		dsSceneTreeNode_markDirty(node->treeNodes[i]);
}

static void dsSceneTransformNode_destroy(dsSceneNode* node)
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

	dsSceneTransformNode* transformNode = DS_ALLOCATE_OBJECT(allocator, dsSceneTransformNode);
	if (!transformNode)
		return NULL;

	dsSceneNode* baseNode = (dsSceneNode*)transformNode;
	if (!dsSceneNode_initialize(baseNode, allocator, dsSceneTransformNode_type(), NULL, 0,
			&dsSceneTransformNode_destroy))
	{
		if (allocator->freeFunc)
			DS_VERIFY(dsAllocator_free(allocator, transformNode));
		return NULL;
	}

	baseNode->setupTreeNodeFunc = &dsSceneTransformNode_setupTreeNode;
	baseNode->shiftNodeFunc = &dsSceneTransformNode_shift;
	if (transform)
		transformNode->transform = *transform;
	else
		dsMatrix44_identity(transformNode->transform);
	return transformNode;
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
