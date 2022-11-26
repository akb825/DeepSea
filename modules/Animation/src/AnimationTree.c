/*
 * Copyright 2022 Aaron Barany
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

#include <DeepSea/Animation/AnimationTree.h>

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/HashTable.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Quaternion.h>

#include <string.h>

typedef struct NamedHashNode
{
	dsHashTableNode node;
	uint32_t index;
} NamedHashNode;

static uint32_t nextID;

static size_t nodeSize(uint32_t childCount)
{
	return DS_ALIGNED_SIZE(sizeof(dsAnimationNode) + sizeof(uint32_t)*childCount);
}

static size_t fullAllocSizeRec(uint32_t* outNodeCount, const dsAnimationBuildNode* node)
{
	++(*outNodeCount);
	if (!node->name || (!node->children && node->childCount > 0))
	{
		return 0;
	}

	size_t curSize = nodeSize(node->childCount);
	for (uint32_t i = 0; i < node->childCount; ++i)
	{
		const dsAnimationBuildNode* child = node->children[i];
		if (!child)
			return 0;

		size_t childSize = fullAllocSizeRec(outNodeCount, child);
		if (childSize == 0)
			return 0;
		curSize += childSize;
	}

	return curSize;
}

static size_t fullAllocSize(uint32_t* outNodeCount, const dsAnimationBuildNode* const* rootNodes,
	uint32_t rootNodeCount)
{
	DS_ASSERT(rootNodeCount > 0);
	*outNodeCount = 0;
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsAnimationTree));
	for (uint32_t i = 0; i < rootNodeCount; ++i)
	{
		const dsAnimationBuildNode* rootNode = rootNodes[i];
		if (!rootNode)
			return 0;

		size_t rootSize = fullAllocSizeRec(outNodeCount, rootNode);
		if (rootSize == 0)
			return 0;
		fullSize += rootSize;
	}

	fullSize += DS_ALIGNED_SIZE(sizeof(dsAnimationNode*)*(*outNodeCount)) +
		DS_ALIGNED_SIZE(sizeof(uint32_t)*rootNodeCount);

	fullSize += DS_ALIGNED_SIZE(sizeof(NamedHashNode))*(*outNodeCount) +
		dsHashTable_fullAllocSize(dsHashTable_tableSize(*outNodeCount));
	return fullSize;
}

static size_t fullAllocSizeJoints(uint32_t* outParentNodes, uint32_t* outRootNodeCount,
	const dsAnimationJointBuildNode* const* nodes, uint32_t nodeCount)
{
	memset(outParentNodes, 0xFF, sizeof(uint32_t)*nodeCount);
	*outRootNodeCount = 0;
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsAnimationTree)) +
		DS_ALIGNED_SIZE(sizeof(dsAnimationNode*)*nodeCount) +
		DS_ALIGNED_SIZE(sizeof(dsMatrix44f)*nodeCount) +
		DS_ALIGNED_SIZE(sizeof(NamedHashNode))*nodeCount +
		dsHashTable_fullAllocSize(dsHashTable_tableSize(nodeCount));
	for (uint32_t i = 0; i < nodeCount; ++i)
	{
		const dsAnimationJointBuildNode* node = nodes[i];
		if (!node || !node->name || (!node->children && node->childCount > 0))
			return 0;

		for (uint32_t j = 0; j < node->childCount; ++j)
		{
			uint32_t child = node->children[j];
			if (child >= nodeCount)
				return 0;

			if (child == i || outParentNodes[child] != DS_NO_ANIMATION_NODE)
			{
				DS_LOG_ERROR_F(DS_ANIMATION_LOG_TAG,
					"Multiple parents for animation joint node '%s'.", node->name);
				return 0;
			}

			outParentNodes[child] = i;
		}

		fullSize += nodeSize(node->childCount);
	}

	for (uint32_t i = 0; i < nodeCount; ++i)
	{
		if (outParentNodes[i] == DS_NO_ANIMATION_NODE)
			++(*outRootNodeCount);
	}

	fullSize += DS_ALIGNED_SIZE(sizeof(uint32_t)*(*outRootNodeCount));
	return fullSize;
}

static uint32_t buildTreeRec(dsAllocator* allocator, uint32_t* nextIndex, uint32_t parent,
	const dsAnimationBuildNode* buildNode, dsAnimationNode** nodes, dsHashTable* nodeTable)
{
	uint32_t index = (*nextIndex)++;
	dsAnimationNode* node = (dsAnimationNode*)dsAllocator_alloc(allocator,
		nodeSize(buildNode->childCount));
	DS_ASSERT(node);
	node->nameID = dsHashString(buildNode->name);
	node->scale = buildNode->scale;
	node->rotation = buildNode->rotation;
	node->translation = buildNode->translation;
	dsMatrix44_identity(node->transform);
	node->parent = parent;
	node->childCount = buildNode->childCount;
	for (uint32_t i = 0; i < buildNode->childCount; ++i)
	{
		uint32_t child = buildTreeRec(allocator, nextIndex, index, buildNode->children[i], nodes,
			nodeTable);
		if (child == DS_NO_ANIMATION_NODE)
			return DS_NO_ANIMATION_NODE;
		node->children[i] = child;
	}

	nodes[index] = node;

	NamedHashNode* hashNode = DS_ALLOCATE_OBJECT(allocator, NamedHashNode);
	DS_ASSERT(hashNode);
	hashNode->index = index;
	if (!dsHashTable_insert(nodeTable, &node->nameID, (dsHashTableNode*)hashNode, NULL))
	{
		DS_LOG_ERROR_F(DS_ANIMATION_LOG_TAG, "Animation tree has multiple nodes named '%s'.",
			buildNode->name);
		return DS_NO_ANIMATION_NODE;
	}

	return index;
}

static void updateTransformsRec(dsAnimationTree* tree, dsAnimationNode* node)
{
	dsMatrix44f scale;
	dsMatrix44f rotation;
	dsMatrix44f translation;
	dsMatrix44f rotateScale;
	dsMatrix44f localTransform;
	dsMatrix44f_makeScale(&scale, node->scale.x, node->scale.y, node->scale.z);
	dsQuaternion4f_toMatrix44(&rotation, &node->rotation);
	dsMatrix44f_makeTranslate(&translation, node->translation.x, node->translation.y,
		node->translation.z);
	dsMatrix44_affineMul(rotateScale, rotation, scale);
	dsMatrix44_affineMul(localTransform, translation, rotateScale);

	if (node->parent == DS_NO_ANIMATION_NODE)
		node->transform = localTransform;
	else
		dsMatrix44_affineMul(node->transform, tree->nodes[node->parent]->transform, localTransform);

	for (uint32_t i = 0; i < node->childCount; ++i)
		updateTransformsRec(tree, tree->nodes[node->children[i]]);
}

dsAnimationTree* dsAnimationTree_create(dsAllocator* allocator,
	const dsAnimationBuildNode* const* rootNodes, uint32_t rootNodeCount)
{
	if (!allocator || !rootNodes || rootNodeCount == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	uint32_t nodeCount;
	size_t fullSize = fullAllocSize(&nodeCount, rootNodes, rootNodeCount);
	if (fullSize == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsAnimationTree* tree = DS_ALLOCATE_OBJECT(&bufferAlloc, dsAnimationTree);
	DS_ASSERT(tree);

	tree->allocator = dsAllocator_keepPointer(allocator);
	tree->id = DS_ATOMIC_FETCH_ADD32(&nextID, 1);
	tree->nodeCount = nodeCount;
	tree->rootNodeCount = rootNodeCount;

	dsAnimationNode** nodes = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsAnimationNode*, nodeCount);
	DS_ASSERT(tree->nodes);
	tree->nodes = nodes;

	uint32_t* rootNodeIndices = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, uint32_t, rootNodeCount);
	DS_ASSERT(rootNodeIndices);
	tree->rootNodes = rootNodeIndices;

	tree->toNodeLocalSpace = NULL;

	uint32_t tableSize = dsHashTable_tableSize(nodeCount);
	dsHashTable* nodeTable = (dsHashTable*)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
		dsHashTable_fullAllocSize(tableSize));
	DS_ASSERT(nodeTable);
	DS_VERIFY(dsHashTable_initialize(nodeTable, tableSize, &dsHashIdentity, &dsHash32Equal));
	tree->nodeTable = nodeTable;

	uint32_t nextIndex = 0;
	for (uint32_t i = 0; i < rootNodeCount; ++i)
	{
		uint32_t root = buildTreeRec(allocator, &nextIndex, DS_NO_ANIMATION_NODE, rootNodes[i],
			nodes, nodeTable);
		if (root == DS_NO_ANIMATION_NODE)
		{
			errno = EINVAL;
			dsAnimationTree_destroy(tree);
			return NULL;
		}
		rootNodeIndices[i] = root;
	}

	return tree;
}

dsAnimationTree* dsAnimationTree_createJoints(dsAllocator* allocator,
	const dsAnimationJointBuildNode* const* nodes, uint32_t nodeCount)
{
	if (!allocator || !nodes || nodeCount == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	uint32_t* parentNodes = DS_ALLOCATE_STACK_OBJECT_ARRAY(uint32_t, nodeCount);
	uint32_t rootNodeCount;
	size_t fullSize = fullAllocSizeJoints(parentNodes, &rootNodeCount, nodes, nodeCount);
	if (fullSize == 0)
	{
		errno = EINVAL;
		return false;
	}

	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsAnimationTree* tree = DS_ALLOCATE_OBJECT(&bufferAlloc, dsAnimationTree);
	DS_ASSERT(tree);

	tree->allocator = dsAllocator_keepPointer(allocator);
	tree->id = DS_ATOMIC_FETCH_ADD32(&nextID, 1);
	tree->nodeCount = nodeCount;
	tree->rootNodeCount = rootNodeCount;

	dsAnimationNode** treeNodes =
		DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsAnimationNode*, nodeCount);
	DS_ASSERT(treeNodes);
	tree->nodes = treeNodes;

	uint32_t* rootNodes = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, uint32_t, rootNodeCount);
	DS_ASSERT(rootNodes);
	tree->rootNodes = rootNodes;

	dsMatrix44f* toNodeLocalSpace = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsMatrix44f, nodeCount);
	DS_ASSERT(toNodeLocalSpace);
	tree->toNodeLocalSpace = toNodeLocalSpace;

	uint32_t tableSize = dsHashTable_tableSize(nodeCount);
	dsHashTable* nodeTable = (dsHashTable*)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
		dsHashTable_fullAllocSize(tableSize));
	DS_ASSERT(nodeTable);
	DS_VERIFY(dsHashTable_initialize(nodeTable, tableSize, &dsHashIdentity, &dsHash32Equal));
	tree->nodeTable = nodeTable;

	uint32_t nextRootNodeIndex = 0;
	for (uint32_t i = 0; i < nodeCount; ++i)
	{
		const dsAnimationJointBuildNode* node = nodes[i];
		dsAnimationNode* treeNode = (dsAnimationNode*)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
			nodeSize(node->childCount));
		DS_ASSERT(treeNode);
		treeNodes[i] = treeNode;

		treeNode->nameID = dsHashString(node->name);
		treeNode->scale = node->scale;
		treeNode->rotation = node->rotation;
		treeNode->translation = node->translation;
		dsMatrix44_identity(treeNode->transform);
		treeNode->parent = parentNodes[i];
		treeNode->childCount = node->childCount;
		memcpy(treeNode->children, node->children, sizeof(uint32_t)*node->childCount);

		if (treeNode->parent == DS_NO_ANIMATION_NODE)
		{
			DS_ASSERT(nextRootNodeIndex < rootNodeCount);
			rootNodes[nextRootNodeIndex++] = i;
		}

		NamedHashNode* hashNode = DS_ALLOCATE_OBJECT(allocator, NamedHashNode);
		DS_ASSERT(hashNode);
		hashNode->index = i;
		if (!dsHashTable_insert(nodeTable, &treeNode->nameID, (dsHashTableNode*)hashNode, NULL))
		{
			DS_LOG_ERROR_F(DS_ANIMATION_LOG_TAG, "Animation tree has multiple nodes named '%s'.",
				node->name);
			errno = EINVAL;
			dsAnimationTree_destroy(tree);
			return NULL;
		}
	}

	return tree;
}

dsAnimationTree* dsAnimationTree_clone(dsAllocator* allocator, const dsAnimationTree* tree)
{
	if (!allocator || !tree)
	{
		errno = EINVAL;
		return NULL;
	}

	uint32_t tableSize = dsHashTable_tableSize(tree->nodeCount);
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsAnimationTree)) +
		DS_ALIGNED_SIZE(sizeof(dsAnimationNode*)*tree->nodeCount) +
		DS_ALIGNED_SIZE(sizeof(uint32_t)*tree->rootNodeCount) +
		DS_ALIGNED_SIZE(sizeof(NamedHashNode))*tree->nodeCount +
		dsHashTable_fullAllocSize(tableSize);
	if (tree->toNodeLocalSpace)
		fullSize += DS_ALIGNED_SIZE(sizeof(dsMatrix44f)*tree->nodeCount);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsAnimationTree* clone = DS_ALLOCATE_OBJECT(&bufferAlloc, dsAnimationTree);
	DS_ASSERT(clone);

	clone->allocator = dsAllocator_keepPointer(allocator);
	clone->id = tree->id;
	clone->nodeCount = tree->nodeCount;
	clone->rootNodeCount = tree->rootNodeCount;

	dsAnimationNode** treeNodes =
		DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsAnimationNode*, tree->nodeCount);
	DS_ASSERT(treeNodes);
	clone->nodes = treeNodes;

	uint32_t* rootNodes = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, uint32_t, tree->rootNodeCount);
	DS_ASSERT(rootNodes);
	memcpy(rootNodes, tree->rootNodes, sizeof(uint32_t)*tree->rootNodeCount);
	clone->rootNodes = rootNodes;

	if (tree->toNodeLocalSpace)
	{
		dsMatrix44f* toNodeLocalSpace = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsMatrix44f,
			tree->nodeCount);
		DS_ASSERT(toNodeLocalSpace);
		memcpy(toNodeLocalSpace, tree->toNodeLocalSpace, sizeof(dsMatrix44f)*tree->nodeCount);
		clone->toNodeLocalSpace = toNodeLocalSpace;
	}
	else
		clone->toNodeLocalSpace = NULL;

	dsHashTable* nodeTable = (dsHashTable*)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
		dsHashTable_fullAllocSize(tableSize));
	DS_ASSERT(nodeTable);
	DS_VERIFY(dsHashTable_initialize(nodeTable, tableSize, &dsHashIdentity, &dsHash32Equal));
	clone->nodeTable = nodeTable;

	for (uint32_t i = 0; i < tree->nodeCount; ++i)
	{
		const dsAnimationNode* node = tree->nodes[i];
		size_t size = nodeSize(node->childCount);
		dsAnimationNode* cloneNode = (dsAnimationNode*)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
			size);
		memcpy(cloneNode, node, size);
		clone->nodes[i] = cloneNode;

		NamedHashNode* hashNode = DS_ALLOCATE_OBJECT(allocator, NamedHashNode);
		DS_ASSERT(hashNode);
		hashNode->index = i;
		DS_VERIFY(dsHashTable_insert(nodeTable, &node->nameID, (dsHashTableNode*)hashNode, NULL));
	}

	return clone;
}

const dsAnimationNode* dsAnimationTree_findNodeName(const dsAnimationTree* tree, const char* name)
{
	if (!tree || !name)
	{
		errno = EINVAL;
		return NULL;
	}

	uint32_t nameID = dsHashString(name);
	NamedHashNode* hashNode = (NamedHashNode*)dsHashTable_find(tree->nodeTable, &nameID);
	if (!hashNode)
	{
		errno = ENOTFOUND;
		return NULL;
	}

	return tree->nodes[hashNode->index];
}

const dsAnimationNode* dsAnimationTree_findNodeID(const dsAnimationTree* tree, uint32_t nameID)
{
	if (!tree)
	{
		errno = EINVAL;
		return NULL;
	}

	NamedHashNode* hashNode = (NamedHashNode*)dsHashTable_find(tree->nodeTable, &nameID);
	if (!hashNode)
	{
		errno = ENOTFOUND;
		return NULL;
	}

	return tree->nodes[hashNode->index];
}

uint32_t dsAnimationTree_findNodeIndexName(const dsAnimationTree* tree, const char* name)
{
	if (!tree || !name)
	{
		errno = EINVAL;
		return DS_NO_ANIMATION_NODE;
	}

	uint32_t nameID = dsHashString(name);
	NamedHashNode* hashNode = (NamedHashNode*)dsHashTable_find(tree->nodeTable, &nameID);
	if (!hashNode)
	{
		errno = ENOTFOUND;
		return DS_NO_ANIMATION_NODE;
	}

	return hashNode->index;
}

uint32_t dsAnimationTree_findNodeIndexID(const dsAnimationTree* tree, uint32_t nameID)
{
	if (!tree)
	{
		errno = EINVAL;
		return DS_NO_ANIMATION_NODE;
	}

	NamedHashNode* hashNode = (NamedHashNode*)dsHashTable_find(tree->nodeTable, &nameID);
	if (!hashNode)
	{
		errno = ENOTFOUND;
		return DS_NO_ANIMATION_NODE;
	}

	return hashNode->index;
}

bool dsAnimationTree_updateTransforms(dsAnimationTree* tree)
{
	if (!tree)
	{
		errno = EINVAL;
		return false;
	}

	for (uint32_t i = 0; i < tree->rootNodeCount; ++i)
		updateTransformsRec(tree, tree->nodes[tree->rootNodes[i]]);

	return true;
}

void dsAnimationTree_destroy(dsAnimationTree* tree)
{
	if (tree && tree->allocator)
		DS_VERIFY(dsAllocator_free(tree->allocator, tree));
}