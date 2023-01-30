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

#include <DeepSea/Geometry/BVH.h>

#include "SpatialStructureShared.h"
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Sort.h>
#include <DeepSea/Geometry/AlignedBox2.h>
#include <DeepSea/Geometry/AlignedBox3.h>
#include <DeepSea/Geometry/Frustum3.h>
#include <string.h>

#define INVALID_NODE ((uint32_t)-1)

typedef struct dsBVHNode
{
	uint32_t leftNode;
	uint32_t rightNode;
	const void* object;
	// Double for worst-case alignment.
	double bounds[];
} dsBVHNode;

struct dsBVH
{
	dsAllocator* allocator;

	dsBVHObjectBoundsFunction objectBoundsFunc;
	void* userData;

	dsGeometryElement element;
	uint8_t axisCount;
	uint8_t nodeSize;
	uint8_t boundsSize;

	dsBVHNode* nodes;
	uint32_t nodeCount;
	uint32_t maxNodes;

	dsBVHNode* tempNodes;
	size_t maxTempNodes;
};

typedef struct SortContext
{
	uint8_t axis;
	uint8_t boundsOffset;
	uint8_t axisCount;
} SortContext;

typedef void (*AddBoxFunction)(void* bounds, const void* otherBounds);
typedef dsIntersectResult (*IntersectFunction)(const void* volume, const void* bounds);

inline static dsBVHNode* getNode(dsBVHNode* nodes, uint8_t nodeSize, uint32_t index)
{
	return (dsBVHNode*)(((uint8_t*)nodes) + index*nodeSize);
}

static int compareBoundsf(const void* left, const void* right, void* context)
{
	const SortContext* sortContext = (const SortContext*)context;
	const float* leftBounds = (const float*)((const uint8_t*)left + sortContext->boundsOffset);
	const float* rightBounds = (const float*)((const uint8_t*)right + sortContext->boundsOffset);

	float leftAverage = (leftBounds[sortContext->axis] +
		leftBounds[sortContext->axisCount + sortContext->axis])*0.5f;
	float rightAverage = (rightBounds[sortContext->axis] +
		rightBounds[sortContext->axisCount + sortContext->axis])*0.5f;

	return DS_CMP(leftAverage, rightAverage);
}

static int compareBoundsd(const void* left, const void* right, void* context)
{
	const SortContext* sortContext = (const SortContext*)context;
	const double* leftBounds = (const double*)((const uint8_t*)left + sortContext->boundsOffset);
	const double* rightBounds = (const double*)((const uint8_t*)right + sortContext->boundsOffset);

	double leftAverage = (leftBounds[sortContext->axis] +
		leftBounds[sortContext->axisCount + sortContext->axis])*0.5;
	double rightAverage = (rightBounds[sortContext->axis] +
		rightBounds[sortContext->axisCount + sortContext->axis])*0.5;

	return DS_CMP(leftAverage, rightAverage);
}

static int compareBoundsi(const void* left, const void* right, void* context)
{
	const SortContext* sortContext = (const SortContext*)context;
	const int* leftBounds = (const int*)((const uint8_t*)left + sortContext->boundsOffset);
	const int* rightBounds = (const int*)((const uint8_t*)right + sortContext->boundsOffset);

	int leftAverage = (leftBounds[sortContext->axis] +
		leftBounds[sortContext->axisCount + sortContext->axis])/2;
	int rightAverage = (rightBounds[sortContext->axis] +
		rightBounds[sortContext->axisCount + sortContext->axis])/2;

	return DS_CMP(leftAverage, rightAverage);
}

// NOTE: Don't bother checking for dsIntersectResult_Inside, since it's expected to be slower unless
// you typically pass very large bounds. Don't use the dsAlignedBox2*_intersects() functions
// directly since it may mess up given that the bool return value is smaller than dsIntersectResult,
// potentially causing undefined results.
static dsIntersectResult intersectBounds2f(const void* volume, const void* bounds)
{
	return (dsIntersectResult)dsAlignedBox2_intersects(*(const dsAlignedBox2f*)volume,
		*(const dsAlignedBox2f*)bounds);
}

static dsIntersectResult intersectBounds2d(const void* volume, const void* bounds)
{
	return (dsIntersectResult)dsAlignedBox2_intersects(*(const dsAlignedBox2d*)volume,
		*(const dsAlignedBox2d*)bounds);
}

static dsIntersectResult intersectBounds2i(const void* volume, const void* bounds)
{
	return (dsIntersectResult)dsAlignedBox2_intersects(*(const dsAlignedBox2i*)volume,
		*(const dsAlignedBox2i*)bounds);
}

static dsIntersectResult intersectBounds3f(const void* volume, const void* bounds)
{
	return (dsIntersectResult)dsAlignedBox3_intersects(*(const dsAlignedBox3f*)volume,
		*(const dsAlignedBox3f*)bounds);
}

static dsIntersectResult intersectBounds3d(const void* volume, const void* bounds)
{
	return (dsIntersectResult)dsAlignedBox3_intersects(*(const dsAlignedBox3d*)volume,
		*(const dsAlignedBox3d*)bounds);
}

static dsIntersectResult intersectBounds3i(const void* volume, const void* bounds)
{
	return (dsIntersectResult)dsAlignedBox3_intersects(*(const dsAlignedBox3i*)volume,
		*(const dsAlignedBox3i*)bounds);
}

static uint32_t buildBVHBalancedRec(dsBVH* bvh, uint32_t start, uint32_t count,
	AddBoxFunction addBoxFunc, MaxAxisFunction maxAxisFunc, dsSortCompareFunction compareFunc,
	uint8_t prevAxis)
{
	uint32_t node = bvh->nodeCount++;
	// Should be guaranteed that reserved nodes is sufficient.
	DS_ASSERT(bvh->nodeCount <= bvh->maxNodes);

	dsBVHNode* bvhNode = getNode(bvh->nodes, bvh->nodeSize, node);
	if (count == 1)
	{
		memcpy(bvhNode, getNode(bvh->tempNodes, bvh->nodeSize, start), bvh->nodeSize);
		return node;
	}

	// Bounds for all current nodes. dsAlignedBox3d is the maximum storage size
	dsAlignedBox3d bounds;
	memcpy(&bounds, getNode(bvh->tempNodes, bvh->nodeSize, start)->bounds, bvh->boundsSize);
	for (uint32_t i = 1; i < count; ++i)
		addBoxFunc(&bounds, getNode(bvh->tempNodes, bvh->nodeSize, start + i)->bounds);

	// Sort based on the maximum dimension. Can skip the sort if the same axis as last call.
	uint8_t maxAxis = maxAxisFunc(&bounds);
	if (maxAxis != prevAxis)
	{
		SortContext context = {maxAxis, (uint8_t)(bvh->nodeSize - bvh->boundsSize), bvh->axisCount};
		dsSort(getNode(bvh->tempNodes, bvh->nodeSize, start), count, bvh->nodeSize, compareFunc,
			&context);
	}

	// Recursively add the nodes.
	uint32_t middle = (uint32_t)count/2;
	uint32_t leftNode = buildBVHBalancedRec(bvh, start, middle, addBoxFunc, maxAxisFunc,
		compareFunc, maxAxis);
	if (leftNode == INVALID_NODE)
		return INVALID_NODE;

	uint32_t rightNode = buildBVHBalancedRec(bvh, start + middle, count - middle, addBoxFunc,
		maxAxisFunc, compareFunc, maxAxis);
	if (rightNode == INVALID_NODE)
		return INVALID_NODE;

	// Recursive operations shouldn't have invalidated the pointer.
	DS_ASSERT(bvhNode == getNode(bvh->nodes, bvh->nodeSize, node));
	bvhNode->leftNode = leftNode;
	bvhNode->rightNode = rightNode;
	bvhNode->object = NULL;
	memcpy(bvhNode->bounds, &bounds, bvh->boundsSize);
	return node;
}

static uint32_t buildBVHRec(dsBVH* bvh, const void* objects, uint32_t start, uint32_t count,
	size_t objectSize, AddBoxFunction addBoxFunc)
{
	uint32_t node = bvh->nodeCount++;
	// Should be guaranteed that reserved nodes is sufficient.
	DS_ASSERT(bvh->nodeCount <= bvh->maxNodes);

	dsBVHNode* bvhNode = getNode(bvh->nodes, bvh->nodeSize, node);
	if (count == 1)
	{
		bvhNode->leftNode = INVALID_NODE;
		bvhNode->rightNode = INVALID_NODE;
		bvhNode->object = dsSpatialStructure_getObject(objects, objectSize, start);
		if (!bvh->objectBoundsFunc(bvhNode->bounds, bvh, bvhNode->object))
			return INVALID_NODE;
		return node;
	}

	// Recursively add the nodes.
	uint32_t middle = (uint32_t)count/2;
	uint32_t leftNode = buildBVHRec(bvh, objects, start, middle, objectSize, addBoxFunc);
	if (leftNode == INVALID_NODE)
		return INVALID_NODE;

	uint32_t rightNode = buildBVHRec(bvh, objects, start + middle, count - middle, objectSize,
		addBoxFunc);
	if (rightNode == INVALID_NODE)
		return INVALID_NODE;

	// Recursive operations shouldn't have invalidated the pointer.
	DS_ASSERT(bvhNode == getNode(bvh->nodes, bvh->nodeSize, node));
	bvhNode->leftNode = leftNode;
	bvhNode->rightNode = rightNode;
	bvhNode->object = NULL;
	memcpy(bvhNode->bounds, getNode(bvh->nodes, bvh->nodeSize, leftNode)->bounds, bvh->boundsSize);
	addBoxFunc(bvhNode->bounds, getNode(bvh->nodes, bvh->nodeSize, rightNode)->bounds);
	return node;
}

static bool updateBVHRec(dsBVH* bvh, dsBVHNode* node, AddBoxFunction addBoxFunc)
{
	if (node->leftNode == INVALID_NODE && node->rightNode == INVALID_NODE)
	{
		DS_ASSERT(node->leftNode == INVALID_NODE && node->rightNode == INVALID_NODE);
		return bvh->objectBoundsFunc(node->bounds, bvh, node->object);
	}

	DS_ASSERT(!node->object);
	dsBVHNode* left = getNode(bvh->nodes, bvh->nodeSize, node->leftNode);
	dsBVHNode* right = getNode(bvh->nodes, bvh->nodeSize, node->rightNode);
	if (!updateBVHRec(bvh, left, addBoxFunc) || !updateBVHRec(bvh, right, addBoxFunc))
		return false;

	memcpy(node->bounds, left->bounds, bvh->boundsSize);
	addBoxFunc(node->bounds, right->bounds);
	return true;
}

// NOTE: bool return value is whether or not to continue traversing
static bool intersectBVHRec(const dsBVH* bvh, uint32_t* count, const dsBVHNode* node,
	const void* volume, dsBVHVisitFunction visitor, void* userData, IntersectFunction intersectFunc,
	bool enclosing)
{
	if (!enclosing)
	{
		dsIntersectResult result = intersectFunc(volume, node->bounds);
		if (result == dsIntersectResult_Outside)
			return true;
		enclosing = result == dsIntersectResult_Inside;
	}

	if (node->leftNode == INVALID_NODE && node->rightNode == INVALID_NODE)
	{
		DS_ASSERT(node->leftNode == INVALID_NODE && node->rightNode == INVALID_NODE);
		++*count;
		if (visitor)
			return visitor(userData, bvh, node->object, volume);
		return true;
	}

	DS_ASSERT(!node->object);
	dsBVHNode* left = getNode(bvh->nodes, bvh->nodeSize, node->leftNode);
	dsBVHNode* right = getNode(bvh->nodes, bvh->nodeSize, node->rightNode);
	if (!intersectBVHRec(bvh, count, left, volume, visitor, userData, intersectFunc, enclosing))
		return false;
	return intersectBVHRec(bvh, count, right, volume, visitor, userData, intersectFunc, enclosing);
}

dsBVH* dsBVH_create(dsAllocator* allocator, uint8_t axisCount, dsGeometryElement element,
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

	dsBVH* bvh = DS_ALLOCATE_OBJECT(allocator, dsBVH);
	if (!bvh)
		return NULL;

	memset(bvh, 0, sizeof(dsBVH));
	bvh->allocator = dsAllocator_keepPointer(allocator);
	bvh->userData = userData;
	bvh->axisCount = axisCount;
	bvh->element = element;
	switch (element)
	{
		case dsGeometryElement_Float:
			bvh->boundsSize = (uint8_t)(sizeof(float)*axisCount*2);
			break;
		case dsGeometryElement_Double:
			bvh->boundsSize = (uint8_t)(sizeof(double)*axisCount*2);
			break;
		case dsGeometryElement_Int:
			bvh->boundsSize = (uint8_t)(sizeof(int)*axisCount*2);
			break;
		default:
			DS_ASSERT(false);
			break;
	}

	size_t nodeSize = sizeof(dsBVHNode) + bvh->boundsSize;
	DS_ASSERT(nodeSize <= UINT8_MAX);
	bvh->nodeSize = (uint8_t)nodeSize;

	return bvh;
}

uint8_t dsBVH_getAxisCount(const dsBVH* bvh)
{
	if (!bvh)
		return 0;

	return bvh->axisCount;
}

dsGeometryElement dsBVH_getElement(const dsBVH* bvh)
{
	if (!bvh)
		return dsGeometryElement_Float;

	return bvh->element;
}

void* dsBVH_getUserData(const dsBVH* bvh)
{
	if (!bvh)
		return NULL;

	return bvh->userData;
}

bool dsBVH_setUserData(dsBVH* bvh, void* userData)
{
	if (!bvh)
	{
		errno = EINVAL;
		return false;
	}

	bvh->userData = userData;
	return true;
}

bool dsBVH_build(dsBVH* bvh, const void* objects, uint32_t objectCount, size_t objectSize,
	dsBVHObjectBoundsFunction objectBoundsFunc, bool balance)
{
	dsBVH_clear(bvh);
	if (!bvh || (!objects && objectCount > 0 && objectSize != DS_GEOMETRY_OBJECT_INDICES) ||
		!objectBoundsFunc)
	{
		errno = EINVAL;
		return false;
	}

	if (objectCount == 0)
		return true;

	AddBoxFunction addBoxFunc;
	MaxAxisFunction maxAxisFunc;
	dsSortCompareFunction compareFunc;
	switch (bvh->element)
	{
		case dsGeometryElement_Float:
			if (bvh->axisCount == 2)
			{
				addBoxFunc = (AddBoxFunction)&dsAlignedBox2f_addBox;
				maxAxisFunc = &dsSpatialStructure_maxAxis2f;
			}
			else
			{
				DS_ASSERT(bvh->axisCount == 3);
				addBoxFunc = (AddBoxFunction)&dsAlignedBox3f_addBox;
				maxAxisFunc = &dsSpatialStructure_maxAxis3f;
			}
			compareFunc = &compareBoundsf;
			break;
		case dsGeometryElement_Double:
			if (bvh->axisCount == 2)
			{
				addBoxFunc = (AddBoxFunction)&dsAlignedBox2d_addBox;
				maxAxisFunc = &dsSpatialStructure_maxAxis2d;
			}
			else
			{
				DS_ASSERT(bvh->axisCount == 3);
				addBoxFunc = (AddBoxFunction)&dsAlignedBox3d_addBox;
				maxAxisFunc = &dsSpatialStructure_maxAxis3d;
			}
			compareFunc = &compareBoundsd;
			break;
		case dsGeometryElement_Int:
			if (bvh->axisCount == 2)
			{
				addBoxFunc = (AddBoxFunction)&dsAlignedBox2i_addBox;
				maxAxisFunc = &dsSpatialStructure_maxAxis2i;
			}
			else
			{
				DS_ASSERT(bvh->axisCount == 3);
				addBoxFunc = (AddBoxFunction)&dsAlignedBox3i_addBox;
				maxAxisFunc = &dsSpatialStructure_maxAxis3i;
			}
			compareFunc = &compareBoundsi;
			break;
		default:
			DS_ASSERT(false);
			return false;
	}

	// Reserve space for BVH. Perfect binary tree, so n = 2*l + 1
	// (where n is number of nodes, l is number of leaves)
	uint32_t nodeCount = objectCount*2 - 1;
	if (!dsResizeableArray_add(bvh->allocator, (void**)&bvh->nodes, &bvh->nodeCount,
			&bvh->maxNodes, bvh->nodeSize, nodeCount))
	{
		return false;
	}
	bvh->nodeCount = 0;

	bvh->objectBoundsFunc = objectBoundsFunc;
	uint32_t rootNode;
	if (balance)
	{
		if (!bvh->tempNodes || objectCount > bvh->maxTempNodes)
		{
			dsAllocator_free(bvh->allocator, bvh->tempNodes);
			bvh->tempNodes = (dsBVHNode*)dsAllocator_alloc(bvh->allocator,
				bvh->nodeSize*objectCount);
			if (!bvh)
				return false;
			bvh->maxTempNodes = objectCount;
		}

		for (uint32_t i = 0; i < objectCount; ++i)
		{
			dsBVHNode* node = getNode(bvh->tempNodes, bvh->nodeSize, i);
			node->object = dsSpatialStructure_getObject(objects, objectSize, i);
			if (!bvh->objectBoundsFunc(node->bounds, bvh, node->object))
				return false;

			node->leftNode = node->rightNode = INVALID_NODE;
		}

		rootNode = buildBVHBalancedRec(bvh, 0, objectCount, addBoxFunc, maxAxisFunc, compareFunc,
			bvh->axisCount);
	}
	else
		rootNode = buildBVHRec(bvh, objects, 0, objectCount, objectSize, addBoxFunc);

	if (rootNode == INVALID_NODE)
	{
		dsBVH_clear(bvh);
		return false;
	}

	DS_ASSERT(rootNode == 0);
	DS_ASSERT(bvh->nodeCount == nodeCount);
	return true;
}

bool dsBVH_update(dsBVH* bvh)
{
	if (!bvh)
	{
		errno = EINVAL;
		return false;
	}

	if (bvh->nodeCount == 0)
		return true;

	AddBoxFunction addBoxFunc;
	switch (bvh->element)
	{
		case dsGeometryElement_Float:
			if (bvh->axisCount == 2)
				addBoxFunc = (AddBoxFunction)&dsAlignedBox2f_addBox;
			else
			{
				DS_ASSERT(bvh->axisCount == 3);
				addBoxFunc = (AddBoxFunction)&dsAlignedBox3f_addBox;
			}
			break;
		case dsGeometryElement_Double:
			if (bvh->axisCount == 2)
				addBoxFunc = (AddBoxFunction)&dsAlignedBox2d_addBox;
			else
			{
				DS_ASSERT(bvh->axisCount == 3);
				addBoxFunc = (AddBoxFunction)&dsAlignedBox3d_addBox;
			}
			break;
		case dsGeometryElement_Int:
			if (bvh->axisCount == 2)
				addBoxFunc = (AddBoxFunction)&dsAlignedBox2i_addBox;
			else
			{
				DS_ASSERT(bvh->axisCount == 3);
				addBoxFunc = (AddBoxFunction)&dsAlignedBox3i_addBox;
			}
			break;
		default:
			DS_ASSERT(false);
			return false;
	}

	return updateBVHRec(bvh, bvh->nodes, addBoxFunc);
}

bool dsBVH_empty(const dsBVH* bvh)
{
	return !bvh || bvh->nodeCount == 0;
}

uint32_t dsBVH_intersectBounds(const dsBVH* bvh, const void* bounds, dsBVHVisitFunction visitor,
	void* userData)
{
	if (!bvh || bvh->nodeCount == 0 || !bounds)
		return 0;

	IntersectFunction intersectFunc;
	switch (bvh->element)
	{
		case dsGeometryElement_Float:
			if (bvh->axisCount == 2)
				intersectFunc = &intersectBounds2f;
			else
			{
				DS_ASSERT(bvh->axisCount == 3);
				intersectFunc = &intersectBounds3f;
			}
			break;
		case dsGeometryElement_Double:
			if (bvh->axisCount == 2)
				intersectFunc = &intersectBounds2d;
			else
			{
				DS_ASSERT(bvh->axisCount == 3);
				intersectFunc = &intersectBounds3d;
			}
			break;
		case dsGeometryElement_Int:
			if (bvh->axisCount == 2)
				intersectFunc = &intersectBounds2i;
			else
			{
				DS_ASSERT(bvh->axisCount == 3);
				intersectFunc = &intersectBounds3i;
			}
			break;
		default:
			DS_ASSERT(false);
			return 0;
	}

	uint32_t count = 0;
	intersectBVHRec(bvh, &count, bvh->nodes, bounds, visitor, userData, intersectFunc, false);
	return count;
}

uint32_t dsBVH_intersectFrustum(const dsBVH* bvh, const void* frustum, dsBVHVisitFunction visitor,
	void* userData)
{
	if (!bvh || bvh->nodeCount == 0 || !frustum || bvh->axisCount != 3 ||
		bvh->element == dsGeometryElement_Int)
	{
		return 0;
	}

	IntersectFunction intersectFunc;
	switch (bvh->element)
	{
		case dsGeometryElement_Float:
			intersectFunc = (IntersectFunction)&dsFrustum3f_intersectAlignedBox;
			break;
		case dsGeometryElement_Double:
			intersectFunc = (IntersectFunction)&dsFrustum3d_intersectAlignedBox;
			break;
		default:
			DS_ASSERT(false);
			return 0;
	}

	uint32_t count = 0;
	intersectBVHRec(bvh, &count, bvh->nodes, frustum, visitor, userData, intersectFunc, false);
	return count;
}

void dsBVH_clear(dsBVH* bvh)
{
	if (!bvh)
		return;

	bvh->nodeCount = 0;
	bvh->objectBoundsFunc = NULL;
}

bool dsBVH_getBounds(void* outBounds, const dsBVH* bvh)
{
	if (!bvh || bvh->nodeCount == 0)
		return false;

	memcpy(outBounds, bvh->nodes->bounds, bvh->boundsSize);
	return true;
}

void dsBVH_destroy(dsBVH* bvh)
{
	if (!bvh || !bvh->allocator)
		return;

	DS_VERIFY(dsAllocator_free(bvh->allocator, bvh->nodes));
	DS_VERIFY(dsAllocator_free(bvh->allocator, bvh->tempNodes));
	DS_VERIFY(dsAllocator_free(bvh->allocator, bvh));
}
