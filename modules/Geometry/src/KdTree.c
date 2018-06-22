/*
 * Copyright 2018 Aaron Barany
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
#include <DeepSea/Geometry/AlignedBox2.h>
#include <DeepSea/Geometry/AlignedBox3.h>
#include <DeepSea/Math/Vector2.h>
#include <DeepSea/Math/Vector3.h>
#include <string.h>

typedef struct dsKdTreeNode
{
	uint32_t leftCount;
	uint32_t rightCount;
	uint8_t axis;
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

typedef void (*AddPointFunction)(void* bounds, const void* point);
typedef void (*MakeInvalidFunction)(void* bounds);

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

static void buildKdTreeRec(dsKdTree* kdTree, uint32_t start, uint32_t count,
	MakeInvalidFunction makeInvalidFunc, AddPointFunction addPointFunc,
	MaxAxisFunction maxAxisFunc, dsSortCompareFunction compareFunc)
{
	if (count == 0)
	{
		dsKdTreeNode* node = getNode(kdTree->nodes, kdTree->nodeSize, start);
		node->leftCount = 0;
		node->rightCount = 0;
		node->axis = 0;
		return;
	}

	// Bounds for all current nodes. dsAlignedBox3d is the maximum storage size
	dsAlignedBox3d bounds;
	makeInvalidFunc(&bounds);
	for (uint32_t i = 0; i < count; ++i)
		addPointFunc(&bounds, getNode(kdTree->nodes, kdTree->nodeSize, i)->point);

	// Sort based on the maximum dimension.
	uint8_t maxAxis = maxAxisFunc(&bounds);
	SortContext context = {maxAxis, (uint8_t)(kdTree->nodeSize - kdTree->pointSize)};
	dsSort(getNode(kdTree->nodes, kdTree->nodeSize, start), count, kdTree->nodeSize, compareFunc,
		&context);

	// Recurse down the middle.
	uint32_t middle = count/2;
	dsKdTreeNode* middleNode = getNode(kdTree->nodes, kdTree->nodeSize, start + middle);
	middleNode->leftCount = middle;
	middleNode->rightCount = middle == count - 1 ? 0 : count - middle - 1;
	middleNode->axis = maxAxis;

	if (middleNode->leftCount > 0)
	{
		buildKdTreeRec(kdTree, start, middleNode->leftCount, makeInvalidFunc, addPointFunc,
			maxAxisFunc, compareFunc);
	}

	if (middleNode->rightCount > 0)
	{
		buildKdTreeRec(kdTree, start + middle + 1, middleNode->rightCount, makeInvalidFunc,
			addPointFunc, maxAxisFunc, compareFunc);
	}
}

static void traverseKdTreeRec(const dsKdTree* kdTree, uint32_t curNode,
	dsKdTreeTraverseFunction traverseFunc, void* userData)
{
	const dsKdTreeNode* node = getNode(kdTree->nodes, kdTree->nodeSize, curNode);
	DS_ASSERT(node->leftCount <= curNode);
	DS_ASSERT(curNode + node->rightCount < kdTree->nodeCount);

	unsigned int side = traverseFunc(userData, kdTree, node->object, node->point, node->axis);
	if (side & dsKdTreeSide_Left && node->leftCount > 0)
	{
		// NOTE: Not the same as curNode - node->leftCount/2 due to integer division.
		traverseKdTreeRec(kdTree, curNode - node->leftCount + node->leftCount/2, traverseFunc,
			userData);
	}
	if (side & dsKdTreeSide_Right && node->rightCount > 0)
		traverseKdTreeRec(kdTree, curNode + 1 + node->rightCount/2, traverseFunc, userData);
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
		DS_LOG_ERROR(DS_GEOMETRY_LOG_TAG, "BVH allocator must support freeing memory.");
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
		return true;

	MakeInvalidFunction makeInvalidFunc;
	AddPointFunction addPointFunc;
	MaxAxisFunction maxAxisFunc;
	dsSortCompareFunction compareFunc;
	switch (kdTree->element)
	{
		case dsGeometryElement_Float:
			if (kdTree->axisCount == 2)
			{
				makeInvalidFunc = (MakeInvalidFunction)&dsAlignedBox2f_makeInvalid;
				addPointFunc = (AddPointFunction)&dsAlignedBox2f_addPoint;
				maxAxisFunc = &dsSpatialStructure_maxAxis2f;
			}
			else
			{
				DS_ASSERT(kdTree->axisCount == 3);
				makeInvalidFunc = (MakeInvalidFunction)&dsAlignedBox3f_makeInvalid;
				addPointFunc = (AddPointFunction)&dsAlignedBox3f_addPoint;
				maxAxisFunc = &dsSpatialStructure_maxAxis3f;
			}
			compareFunc = &comparePointf;
			break;
		case dsGeometryElement_Double:
			if (kdTree->axisCount == 2)
			{
				makeInvalidFunc = (MakeInvalidFunction)&dsAlignedBox2d_makeInvalid;
				addPointFunc = (AddPointFunction)&dsAlignedBox2d_addPoint;
				maxAxisFunc = &dsSpatialStructure_maxAxis2d;
			}
			else
			{
				DS_ASSERT(kdTree->axisCount == 3);
				makeInvalidFunc = (MakeInvalidFunction)&dsAlignedBox3d_makeInvalid;
				addPointFunc = (AddPointFunction)&dsAlignedBox3d_addPoint;
				maxAxisFunc = &dsSpatialStructure_maxAxis3d;
			}
			compareFunc = &comparePointd;
			break;
		case dsGeometryElement_Int:
			if (kdTree->axisCount == 2)
			{
				makeInvalidFunc = (MakeInvalidFunction)&dsAlignedBox2i_makeInvalid;
				addPointFunc = (AddPointFunction)&dsAlignedBox2i_addPoint;
				maxAxisFunc = &dsSpatialStructure_maxAxis2i;
			}
			else
			{
				DS_ASSERT(kdTree->axisCount == 3);
				makeInvalidFunc = (MakeInvalidFunction)&dsAlignedBox2d_makeInvalid;
				addPointFunc = (AddPointFunction)&dsAlignedBox3i_addPoint;
				maxAxisFunc = &dsSpatialStructure_maxAxis3i;
			}
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
	}

	buildKdTreeRec(kdTree, 0, objectCount, makeInvalidFunc, addPointFunc, maxAxisFunc, compareFunc);
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
		traverseKdTreeRec(kdTree, kdTree->nodeCount/2, traverseFunc, userData);
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
