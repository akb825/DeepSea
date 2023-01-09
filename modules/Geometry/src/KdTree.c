/*
 * Copyright 2018-2023 Aaron Barany
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

#include <float.h>
#include <string.h>

#define INVALID_NODE ((uint32_t)-1)

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

	uint32_t rootNode;

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

	return DS_CMP(leftPoint[sortContext->axis], rightPoint[sortContext->axis]);
}

static int comparePointd(const void* left, const void* right, void* context)
{
	const SortContext* sortContext = (const SortContext*)context;
	const double* leftPoint = (const double*)((const uint8_t*)left + sortContext->pointOffset);
	const double* rightPoint = (const double*)((const uint8_t*)right + sortContext->pointOffset);

	return DS_CMP(leftPoint[sortContext->axis], rightPoint[sortContext->axis]);
}

static int comparePointi(const void* left, const void* right, void* context)
{
	const SortContext* sortContext = (const SortContext*)context;
	const int* leftPoint = (const int*)((const uint8_t*)left + sortContext->pointOffset);
	const int* rightPoint = (const int*)((const uint8_t*)right + sortContext->pointOffset);

	return DS_CMP(leftPoint[sortContext->axis], rightPoint[sortContext->axis]);
}

static uint32_t buildKdTreeBalancedRec(dsKdTree* kdTree, uint32_t start, uint32_t count,
	uint8_t axis, dsSortCompareFunction compareFunc)
{
	DS_ASSERT(count > 0);

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

static const void* nearestNeighborRecFloat(const dsKdTree* kdTree, uint32_t curNode,
	const float* point, float* curDistance, uint8_t axis)
{
	const dsKdTreeNode* node = getNode(kdTree->nodes, kdTree->nodeSize, curNode);
	const float* nodePoint = (const float*)node->point;

	float distance = 0;
	for (uint8_t i = 0; i < kdTree->axisCount; ++i)
	{
		float diff = point[i] - nodePoint[i];
		distance += dsPow2(diff);
	}
	distance = sqrtf(distance);

	if (node->leftNode == INVALID_NODE && node->rightNode == INVALID_NODE)
	{
		if (distance < *curDistance)
		{
			*curDistance = distance;
			return node->object;
		}
		else
			return NULL;
	}

	// First follow the proper child to find the closest leaf.
	uint8_t nextAxis = (uint8_t)((axis + 1) % kdTree->axisCount);
	bool leftPath = point[axis] < nodePoint[axis];
	const void* closest = NULL;
	if (leftPath)
	{
		if (node->leftNode != INVALID_NODE)
			closest = nearestNeighborRecFloat(kdTree, node->leftNode, point, curDistance, nextAxis);
	}
	else
	{
		if (node->rightNode != INVALID_NODE)
		{
			closest = nearestNeighborRecFloat(kdTree, node->rightNode, point, curDistance,
				nextAxis);
		}
	}

	// Check the current node and other path if it's closer than the leaf.
	if (distance < *curDistance)
	{
		*curDistance = distance;
		closest = node->object;
	}

	// Check the path opposite of the previous one.
	if (leftPath)
	{
		if (node->rightNode != INVALID_NODE && nodePoint[axis] <= point[axis] + *curDistance)
		{
			const void* pathClosest = nearestNeighborRecFloat(kdTree, node->rightNode, point,
				curDistance, nextAxis);
			if (pathClosest)
				closest = pathClosest;
		}
	}
	else
	{
		if (node->leftNode != INVALID_NODE && nodePoint[axis] >= point[axis] - *curDistance)
		{
			const void* pathClosest = nearestNeighborRecFloat(kdTree, node->leftNode, point,
				curDistance, nextAxis);
			if (pathClosest)
				closest = pathClosest;
		}
	}

	return closest;
}

static const void* nearestNeighborRecDouble(const dsKdTree* kdTree, uint32_t curNode,
	const double* point, double* curDistance, uint8_t axis)
{
	const dsKdTreeNode* node = getNode(kdTree->nodes, kdTree->nodeSize, curNode);
	const double* nodePoint = node->point;

	double distance = 0;
	for (uint8_t i = 0; i < kdTree->axisCount; ++i)
	{
		double diff = point[i] - nodePoint[i];
		distance += dsPow2(diff);
	}
	distance = sqrt(distance);

	if (node->leftNode == INVALID_NODE && node->rightNode == INVALID_NODE)
	{
		if (distance < *curDistance)
		{
			*curDistance = distance;
			return node->object;
		}
		else
			return NULL;
	}

	// First follow the proper child to find the closest leaf.
	uint8_t nextAxis = (uint8_t)((axis + 1) % kdTree->axisCount);
	bool leftPath = point[axis] < nodePoint[axis];
	const void* closest = NULL;
	if (leftPath)
	{
		if (node->leftNode != INVALID_NODE)
		{
			closest = nearestNeighborRecDouble(kdTree, node->leftNode, point, curDistance,
				nextAxis);
		}
	}
	else
	{
		if (node->rightNode != INVALID_NODE)
		{
			closest = nearestNeighborRecDouble(kdTree, node->rightNode, point, curDistance,
				nextAxis);
		}
	}

	// Check the current node and other path if it's closer than the leaf.
	if (distance < *curDistance)
	{
		*curDistance = distance;
		closest = node->object;
	}

	// Check the path opposite of the previous one.
	if (leftPath)
	{
		if (node->rightNode != INVALID_NODE && nodePoint[axis] <= point[axis] + *curDistance)
		{
			const void* pathClosest = nearestNeighborRecDouble(kdTree, node->rightNode, point,
				curDistance, nextAxis);
			if (pathClosest)
				closest = pathClosest;
		}
	}
	else
	{
		if (node->leftNode != INVALID_NODE && nodePoint[axis] >= point[axis] - *curDistance)
		{
			const void* pathClosest = nearestNeighborRecDouble(kdTree, node->leftNode, point,
				curDistance, nextAxis);
			if (pathClosest)
				closest = pathClosest;
		}
	}

	return closest;
}

static const void* nearestNeighborRecInt(const dsKdTree* kdTree, uint32_t curNode,
	const int* point, double* curDistance, uint8_t axis)
{
	const dsKdTreeNode* node = getNode(kdTree->nodes, kdTree->nodeSize, curNode);
	const int* nodePoint = (const int*)node->point;

	double distance = 0;
	for (uint8_t i = 0; i < kdTree->axisCount; ++i)
	{
		double diff = point[i] - nodePoint[i];
		distance += dsPow2(diff);
	}
	distance = sqrt(distance);

	if (node->leftNode == INVALID_NODE && node->rightNode == INVALID_NODE)
	{
		if (distance < *curDistance)
		{
			*curDistance = distance;
			return node->object;
		}
		else
			return NULL;
	}

	// First follow the proper child to find the closest leaf.
	uint8_t nextAxis = (uint8_t)((axis + 1) % kdTree->axisCount);
	bool leftPath = point[axis] < nodePoint[axis];
	const void* closest = NULL;
	if (leftPath)
	{
		if (node->leftNode != INVALID_NODE)
			closest = nearestNeighborRecInt(kdTree, node->leftNode, point, curDistance, nextAxis);
	}
	else
	{
		if (node->rightNode != INVALID_NODE)
			closest = nearestNeighborRecInt(kdTree, node->rightNode, point, curDistance, nextAxis);
	}

	// Check the current node and other path if it's closer than the leaf.
	if (distance < *curDistance)
	{
		*curDistance = distance;
		closest = node->object;
	}

	// Check the path opposite of the previous one.
	if (leftPath)
	{
		if (node->rightNode != INVALID_NODE && nodePoint[axis] <= point[axis] + *curDistance)
		{
			const void* pathClosest = nearestNeighborRecInt(kdTree, node->rightNode, point,
				curDistance, nextAxis);
			if (pathClosest)
				closest = pathClosest;
		}
	}
	else
	{
		if (node->leftNode != INVALID_NODE && nodePoint[axis] >= point[axis] - *curDistance)
		{
			const void* pathClosest = nearestNeighborRecInt(kdTree, node->leftNode, point,
				curDistance, nextAxis);
			if (pathClosest)
				closest = pathClosest;
		}
	}

	return closest;
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
	dsKdTreeObjectPointFunction objectPointFunc)
{
	dsKdTree_clear(kdTree);
	if (!kdTree || (!objects && objectCount > 0 && objectSize != DS_GEOMETRY_OBJECT_INDICES) ||
		!objectPointFunc)
	{
		errno = EINVAL;
		return false;
	}

	if (objectCount == 0)
	{
		kdTree->rootNode = 0;
		return true;
	}

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

	kdTree->rootNode = buildKdTreeBalancedRec(kdTree, 0, objectCount, 0, compareFunc);
	return true;
}

const void* dsKdTree_nearestNeighbor(const dsKdTree* kdTree, const void* point)
{
	if (!kdTree || !point || kdTree->nodeCount == 0)
		return NULL;

	switch (kdTree->element)
	{
		case dsGeometryElement_Float:
		{
			float distance = FLT_MAX;
			return nearestNeighborRecFloat(kdTree, kdTree->rootNode, (const float*)point,
				&distance, 0);
		}
		case dsGeometryElement_Double:
		{
			double distance = DBL_MAX;
			return nearestNeighborRecDouble(kdTree, kdTree->rootNode, (const double*)point,
				&distance, 0);
		}
		case dsGeometryElement_Int:
		{
			double distance = DBL_MAX;
			return nearestNeighborRecInt(kdTree, kdTree->rootNode, (const int*)point,
				&distance, 0);
		}
	}

	DS_ASSERT(false);
	return NULL;
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
		traverseKdTreeRec(kdTree, kdTree->rootNode, traverseFunc, userData, 0);
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
