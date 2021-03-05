/*
 * Copyright 2018-2021 Aaron Barany
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

#include <DeepSea/Geometry/KdTree.h>

#include "SpatialStructureShared.h"
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Sort.h>
#include <DeepSea/Math/Vector2.h>
#include <DeepSea/Math/Vector3.h>
#include <string.h>

#define INVALID_NODE (uint32_t)-1

typedef struct dsKdTreeNode
{
	uint32_t leftNode;
	uint32_t rightNode;
	const void* object;
	// Double for worst-case alignment.
	double point[];
} dsKdTreeNode;

struct dsKdTree
{
	dsAllocator* allocator;

	void* userData;

	dsGeometryElement element;
	uint8_t axisCount;
	uint8_t nodeSize;
	uint8_t pointSize;

	dsKdTreeNode* nodes;
	uint32_t nodeCount;
	uint32_t maxNodes;
};

typedef struct SortContext
{
	uint8_t axis;
	uint8_t pointOffset;
} SortContext;

inline static dsKdTreeNode* getNode(dsKdTreeNode* nodes, uint8_t nodeSize, uint32_t index)
{
	return (dsKdTreeNode*)(((uint8_t*)nodes) + index*nodeSize);
}

static int comparePointf(const void* left, const void* right, void* context)
{
	const SortContext* sortContext = (const SortContext*)context;
	const float* leftPoint = (const float*)((const uint8_t*)left + sortContext->pointOffset);
	const float* rightPoint = (const float*)((const uint8_t*)right + sortContext->pointOffset);

	float leftVal = leftPoint[sortContext->axis];
	float rightVal = rightPoint[sortContext->axis];

	if (leftVal < rightVal)
		return -1;
	else if (leftVal > rightVal)
		return 1;
	return 0;
}

static int comparePointd(const void* left, const void* right, void* context)
{
	const SortContext* sortContext = (const SortContext*)context;
	const double* leftPoint = (const double*)((const uint8_t*)left + sortContext->pointOffset);
	const double* rightPoint = (const double*)((const uint8_t*)right + sortContext->pointOffset);

	double leftVal = leftPoint[sortContext->axis];
	double rightVal = rightPoint[sortContext->axis];

	if (leftVal < rightVal)
		return -1;
	else if (leftVal > rightVal)
		return 1;
	return 0;
}

static int comparePointi(const void* left, const void* right, void* context)
{
	const SortContext* sortContext = (const SortContext*)context;
	const int* leftPoint = (const int*)((const uint8_t*)left + sortContext->pointOffset);
	const int* rightPoint = (const int*)((const uint8_t*)right + sortContext->pointOffset);

	return leftPoint[sortContext->axis] - rightPoint[sortContext->axis];
}

static uint32_t buildKdTreeBalancedRec(dsKdTree* kdTree, uint32_t start, uint32_t count,
	uint8_t axis, dsSortCompareFunction compareFunc)
{
	if (count == 0)
	{
		dsKdTreeNode* node = getNode(kdTree->nodes, kdTree->nodeSize, start);
		node->leftNode = INVALID_NODE;
		node->rightNode = INVALID_NODE;
		return start;
	}

	// Sort based on the maximum dimension.
	SortContext context = {axis, (uint8_t)(kdTree->nodeSize - kdTree->pointSize)};
	dsSort(getNode(kdTree->nodes, kdTree->nodeSize, start), count, kdTree->nodeSize, compareFunc,
		&context);

	// Recurse down the middle.
	uint32_t middle = count/2;
	uint32_t middleNodeIndex = start + middle;
	dsKdTreeNode* middleNode = getNode(kdTree->nodes, kdTree->nodeSize, middleNodeIndex);
	uint32_t leftCount = middle;
	uint32_t rightCount = middle == count - 1 ? 0 : count - middle - 1;

	uint8_t nextAxis = (uint8_t)((axis + 1) % kdTree->axisCount);
	if (leftCount > 0)
	{
		middleNode->leftNode = buildKdTreeBalancedRec(kdTree, start, leftCount, nextAxis,
			compareFunc);
	}

	if (rightCount > 0)
	{
		middleNode->rightNode = buildKdTreeBalancedRec(kdTree, start + middle + 1, rightCount,
			nextAxis, compareFunc);
	}

	return middleNodeIndex;
}

static void insertKdTreeNodeRec(dsKdTree* kdTree, uint32_t current,	uint32_t newNodeIndex,
	dsKdTreeNode* newNode, uint8_t axis, dsSortCompareFunction compareFunc)
{
	dsKdTreeNode* node = getNode(kdTree->nodes, kdTree->nodeSize, current);
	SortContext context = {axis, (uint8_t)(kdTree->nodeSize - kdTree->pointSize)};
	uint8_t nextAxis = (uint8_t)((axis + 1) % kdTree->axisCount);
	if (compareFunc(newNode, node, &context) < 0)
	{
		if (node->leftNode == INVALID_NODE)
			node->leftNode = newNodeIndex;
		else
		{
			insertKdTreeNodeRec(kdTree, node->leftNode, newNodeIndex, newNode, nextAxis,
				compareFunc);
		}
	}
	else
	{
		if (node->rightNode == INVALID_NODE)
			node->rightNode = newNodeIndex;
		else
		{
			insertKdTreeNodeRec(kdTree, node->rightNode, newNodeIndex, newNode, nextAxis,
				compareFunc);
		}
	}
}

static bool traverseKdTreeRec(const dsKdTree* kdTree, uint32_t curNode,
	dsKdTreeTraverseFunction traverseFunc, void* userData, uint8_t axis)
{
	const dsKdTreeNode* node = getNode(kdTree->nodes, kdTree->nodeSize, curNode);

	dsKdTreeSide side = traverseFunc(userData, kdTree, node->object, node->point, axis);
	if (side & dsKdTreeSide_Stop)
		return false;

	uint8_t nextAxis = (uint8_t)((axis + 1) % kdTree->axisCount);
	if ((side & dsKdTreeSide_Left) && node->leftNode != INVALID_NODE)
	{
		if (!traverseKdTreeRec(kdTree, node->leftNode, traverseFunc, userData, nextAxis))
			return false;
	}
	if ((side & dsKdTreeSide_Right) && node->rightNode != INVALID_NODE)
		return traverseKdTreeRec(kdTree, node->rightNode, traverseFunc, userData, nextAxis);
	return true;
}

dsKdTree* dsKdTree_create(dsAllocator* allocator, uint8_t axisCount, dsGeometryElement element,
	void* userData)
{
	if (!allocator || axisCount < 2 || axisCount > 3 || element < dsGeometryElement_Float ||
		element > dsGeometryElement_Int)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_GEOMETRY_LOG_TAG, "Kd tree allocator must support freeing memory.");
		return NULL;
	}

	dsKdTree* kdTree = DS_ALLOCATE_OBJECT(allocator, dsKdTree);
	if (!kdTree)
		return NULL;

	memset(kdTree, 0, sizeof(dsKdTree));
	kdTree->allocator = dsAllocator_keepPointer(allocator);
	kdTree->userData = userData;
	kdTree->axisCount = axisCount;
	kdTree->element = element;
	switch (element)
	{
		case dsGeometryElement_Float:
			kdTree->pointSize = (uint8_t)(sizeof(float)*axisCount);
			break;
		case dsGeometryElement_Double:
			kdTree->pointSize = (uint8_t)(sizeof(double)*axisCount);
			break;
		case dsGeometryElement_Int:
			kdTree->pointSize = (uint8_t)(sizeof(int)*axisCount);
			break;
		default:
			DS_ASSERT(false);
			break;
	}

	size_t nodeSize = sizeof(dsKdTreeNode) + kdTree->pointSize;
	DS_ASSERT(nodeSize <= UINT8_MAX);
	kdTree->nodeSize = (uint8_t)nodeSize;

	return kdTree;
}

uint8_t dsKdTree_getAxisCount(const dsKdTree* kdTree)
{
	if (!kdTree)
		return 0;

	return kdTree->axisCount;
}

dsGeometryElement dsKdTree_getElement(const dsKdTree* kdTree)
{
	if (!kdTree)
		return dsGeometryElement_Float;

	return kdTree->element;
}

void* dsKdTree_getUserData(const dsKdTree* kdTree)
{
	if (!kdTree)
		return NULL;

	return kdTree->userData;
}

void dsKdTree_setUserData(dsKdTree* kdTree, void* userData)
{
	if (kdTree)
		kdTree->userData = userData;
}

bool dsKdTree_build(dsKdTree* kdTree, const void* objects, uint32_t objectCount, size_t objectSize,
	dsKdTreeObjectPointFunction objectPointFunc, bool balance)
{
	dsKdTree_clear(kdTree);
	if (!kdTree || (!objects && objectCount > 0 && objectSize != DS_GEOMETRY_OBJECT_INDICES) ||
		!objectPointFunc)
	{
		errno = EINVAL;
		return false;
	}

	if (objectCount == 0)
		return true;

	dsSortCompareFunction compareFunc;
	switch (kdTree->element)
	{
		case dsGeometryElement_Float:
			compareFunc = &comparePointf;
			break;
		case dsGeometryElement_Double:
			compareFunc = &comparePointd;
			break;
		case dsGeometryElement_Int:
			compareFunc = &comparePointi;
			break;
		default:
			DS_ASSERT(false);
			return false;
	}

	if (!dsResizeableArray_add(kdTree->allocator, (void**)&kdTree->nodes, &kdTree->nodeCount,
			&kdTree->maxNodes, kdTree->nodeSize, objectCount))
	{
		return false;
	}

	for (uint32_t i = 0; i < objectCount; ++i)
	{
		dsKdTreeNode* node = getNode(kdTree->nodes, kdTree->nodeSize, i);
		const void* object = dsSpatialStructure_getObject(objects, objectSize, i);
		if (!objectPointFunc(node->point, kdTree, object))
		{
			dsKdTree_clear(kdTree);
			return false;
		}

		node->object = object;
		node->leftNode = INVALID_NODE;
		node->rightNode = INVALID_NODE;
	}

	if (balance)
		buildKdTreeBalancedRec(kdTree, 0, objectCount, 0, compareFunc);
	else
	{
		// Keep the root node consistent with the balanced case.
		uint32_t root = kdTree->nodeCount/2;
		for (uint32_t i = 0; i < root; ++i)
		{
			insertKdTreeNodeRec(kdTree, root, i, getNode(kdTree->nodes, kdTree->nodeSize, i), 0,
				compareFunc);
		}
		for (uint32_t i = root + 1; i < objectCount; ++i)
		{
			insertKdTreeNodeRec(kdTree, root, i, getNode(kdTree->nodes, kdTree->nodeSize, i), 0,
				compareFunc);
		}
	}
	return true;
}

bool dsKdTree_traverse(const dsKdTree* kdTree, dsKdTreeTraverseFunction traverseFunc,
	void* userData)
{
	if (!kdTree || !traverseFunc)
	{
		errno = EINVAL;
		return false;
	}

	if (kdTree->nodeCount > 0)
		traverseKdTreeRec(kdTree, kdTree->nodeCount/2, traverseFunc, userData, 0);
	return true;
}

void dsKdTree_clear(dsKdTree* kdTree)
{
	if (!kdTree)
		return;

	kdTree->nodeCount = 0;
}

void dsKdTree_destroy(dsKdTree* kdTree)
{
	if (!kdTree || !kdTree->allocator)
		return;

	DS_VERIFY(dsAllocator_free(kdTree->allocator, kdTree->nodes));
	DS_VERIFY(dsAllocator_free(kdTree->allocator, kdTree));
}
