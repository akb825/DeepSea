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

#include <DeepSea/Geometry/BVH.h>

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Sort.h>
#include <DeepSea/Geometry/AlignedBox2.h>
#include <DeepSea/Geometry/AlignedBox3.h>
#include <string.h>

#define INVALID_NODE (uint32_t)-1

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
typedef uint8_t (*MaxAxisFunction)(const void* bounds);
typedef bool (*IntersectFunction)(const void* bounds, const void* otherBounds);

inline static const void* getObject(const void* objects, size_t objectSize, size_t index)
{
	if (objectSize == DS_BVH_OBJECT_POINTERS)
		return ((const void**)objects)[index];
	else if (objectSize == DS_BVH_OBJECT_INDICES)
		return (const void*)index;
	return ((const uint8_t*)objects) + objectSize*index;
}

inline static dsBVHNode* getNode(dsBVHNode* nodes, uint8_t nodeSize, size_t index)
{
	return (dsBVHNode*)(((uint8_t*)nodes) + index*nodeSize);
}

static uint8_t maxAxis2f(const void* bounds)
{
	const dsAlignedBox2f* realBounds = (const dsAlignedBox2f*)bounds;
	dsVector2f extents;
	dsAlignedBox2_extents(extents, *realBounds);
	if (extents.x >= extents.y)
		return 0;
	return 1;
}

static uint8_t maxAxis3f(const void* bounds)
{
	const dsAlignedBox3f* realBounds = (const dsAlignedBox3f*)bounds;
	dsVector3f extents;
	dsAlignedBox3_extents(extents, *realBounds);
	if (extents.x >= extents.y && extents.x >= extents.z)
		return 0;
	if (extents.y >= extents.x && extents.y >= extents.z)
		return 1;
	return 2;
}

static int compareBoundsf(const void* left, const void* right, void* context)
{
	uint8_t axis = ((SortContext*)context)->axis;
	uint8_t axisCount = ((SortContext*)context)->axisCount;
	const float* leftBounds = (const float*)left;
	const float* rightBounds = (const float*)right;

	float leftAverage = (leftBounds[axis] + leftBounds[axisCount + axis])*0.5f;
	float rightAverage = (rightBounds[axis] + rightBounds[axisCount + axis])*0.5f;
	if (leftAverage < rightAverage)
		return -1;
	else if (leftAverage > rightAverage)
		return 1;
	return 0;
}

static uint8_t maxAxis2d(const void* bounds)
{
	const dsAlignedBox2d* realBounds = (const dsAlignedBox2d*)bounds;
	dsVector2d extents;
	dsAlignedBox2_extents(extents, *realBounds);
	if (extents.x >= extents.y)
		return 0;
	return 1;
}

static uint8_t maxAxis3d(const void* bounds)
{
	const dsAlignedBox3d* realBounds = (const dsAlignedBox3d*)bounds;
	dsVector3d extents;
	dsAlignedBox3_extents(extents, *realBounds);
	if (extents.x >= extents.y && extents.x >= extents.z)
		return 0;
	if (extents.y >= extents.x && extents.y >= extents.z)
		return 1;
	return 2;
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

	if (leftAverage < rightAverage)
		return -1;
	else if (leftAverage > rightAverage)
		return 1;
	return 0;
}

static uint8_t maxAxis2i(const void* bounds)
{
	const dsAlignedBox2i* realBounds = (const dsAlignedBox2i*)bounds;
	dsVector2i extents;
	dsAlignedBox2_extents(extents, *realBounds);
	if (extents.x >= extents.y)
		return 0;
	return 1;
}

static uint8_t maxAxis3i(const void* bounds)
{
	const dsAlignedBox3i* realBounds = (const dsAlignedBox3i*)bounds;
	dsVector3i extents;
	dsAlignedBox3_extents(extents, *realBounds);
	if (extents.x >= extents.y && extents.x >= extents.z)
		return 0;
	if (extents.y >= extents.x && extents.y >= extents.z)
		return 1;
	return 2;
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

	return leftAverage - rightAverage;
}

static uint32_t buildBVHBalancedRec(dsBVH* bvh, size_t start, size_t count,
	AddBoxFunction addBoxFunc, MaxAxisFunction maxAxisFunc, dsSortCompareFunction compareFunc)
{
	uint32_t node = bvh->nodeCount;
	if (!dsResizeableArray_add(bvh->allocator, (void**)&bvh->nodes, &bvh->nodeCount, &bvh->maxNodes,
		bvh->nodeSize, 1))
	{
		return INVALID_NODE;
	}

	dsBVHNode* bvhNode = getNode(bvh->nodes, bvh->nodeSize, node);
	if (count == 1)
	{
		memcpy(bvhNode, getNode(bvh->tempNodes, bvh->nodeSize, start), bvh->nodeSize);
		return node;
	}

	// Bounds for all current nodes. dsAlignedBox3d is the maximum storage size
	dsAlignedBox3d bounds;
	memcpy(&bounds, getNode(bvh->tempNodes, bvh->nodeSize, start)->bounds, bvh->boundsSize);
	for (size_t i = 1; i < count; ++i)
		addBoxFunc(&bounds, getNode(bvh->tempNodes, bvh->nodeSize, start + i)->bounds);

	// Sort based on the maximum dimension.
	uint8_t maxAxis = maxAxisFunc(&bounds);
	SortContext context = {maxAxis, (uint8_t)(bvh->nodeSize - bvh->boundsSize), bvh->axisCount};
	dsSort(getNode(bvh->tempNodes, bvh->nodeSize, start), count, bvh->nodeSize, compareFunc,
		&context);

	// Recursively add the nodes.
	uint32_t middle = (uint32_t)count/2;
	uint32_t leftNode = buildBVHBalancedRec(bvh, start, middle, addBoxFunc, maxAxisFunc,
		compareFunc);
	if (leftNode == INVALID_NODE)
		return INVALID_NODE;

	uint32_t rightNode = buildBVHBalancedRec(bvh, start + middle, count - middle, addBoxFunc,
		maxAxisFunc, compareFunc);
	if (rightNode == INVALID_NODE)
		return INVALID_NODE;

	// Reset pointer due to possible re-allocations of the array.
	bvhNode = getNode(bvh->nodes, bvh->nodeSize, node);
	bvhNode->leftNode = leftNode;
	bvhNode->rightNode = rightNode;
	bvhNode->object = NULL;
	memcpy(bvhNode->bounds, &bounds, bvh->boundsSize);
	return node;
}

static uint32_t buildBVHRec(dsBVH* bvh, const void* objects, size_t start, size_t count,
	size_t objectSize, AddBoxFunction addBoxFunc)
{
	uint32_t node = bvh->nodeCount;
	if (!dsResizeableArray_add(bvh->allocator, (void**)&bvh->nodes, &bvh->nodeCount, &bvh->maxNodes,
		bvh->nodeSize, 1))
	{
		return INVALID_NODE;
	}

	dsBVHNode* bvhNode = getNode(bvh->nodes, bvh->nodeSize, node);
	if (count == 1)
	{
		bvhNode->leftNode = INVALID_NODE;
		bvhNode->rightNode = INVALID_NODE;
		bvhNode->object = getObject(objects, objectSize, start);
		if (!bvh->objectBoundsFunc(bvhNode->bounds, bvh, bvhNode->object))
			return INVALID_NODE;
		return node;
	}

	// Bounds for all current nodes. dsAlignedBox3d is the maximum storage size
	dsAlignedBox3d bounds;
	if (!bvh->objectBoundsFunc(&bounds, bvh, getObject(objects, objectSize, start)))
		return INVALID_NODE;

	for (size_t i = 1; i < count; ++i)
	{
		dsAlignedBox3d curBounds;
		if (!bvh->objectBoundsFunc(&curBounds, bvh, getObject(objects, objectSize, start + i)))
			return INVALID_NODE;

		addBoxFunc(&bounds, &curBounds);
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

	// Reset pointer due to possible re-allocations of the array.
	bvhNode = getNode(bvh->nodes, bvh->nodeSize, node);
	bvhNode->leftNode = leftNode;
	bvhNode->rightNode = rightNode;
	bvhNode->object = NULL;
	memcpy(bvhNode->bounds, &bounds, bvh->boundsSize);
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
	const void* bounds, dsBVHVisitFunction visitor, void* userData, IntersectFunction intersectFunc)
{
	if (!intersectFunc(bounds, node->bounds))
		return true;

	if (node->leftNode == INVALID_NODE && node->rightNode == INVALID_NODE)
	{
		DS_ASSERT(node->leftNode == INVALID_NODE && node->rightNode == INVALID_NODE);
		++*count;
		if (visitor)
			return visitor(userData, bvh, node->object, bounds);
		return true;
	}

	DS_ASSERT(!node->object);
	dsBVHNode* left = getNode(bvh->nodes, bvh->nodeSize, node->leftNode);
	dsBVHNode* right = getNode(bvh->nodes, bvh->nodeSize, node->rightNode);
	if (!intersectBVHRec(bvh, count, left, bounds, visitor, userData, intersectFunc))
		return false;
	return intersectBVHRec(bvh, count, right, bounds, visitor, userData, intersectFunc);
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

void dsBVH_setUserData(dsBVH* bvh, void* userData)
{
	if (bvh)
		bvh->userData = userData;
}

bool dsBVH_build(dsBVH* bvh, const void* objects, uint32_t objectCount, size_t objectSize,
	dsBVHObjectBoundsFunction objectBoundsFunc, bool balance)
{
	dsBVH_clear(bvh);
	if (!bvh || (!objects && objectCount > 0 && objectSize != DS_BVH_OBJECT_INDICES) ||
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
				maxAxisFunc = &maxAxis2f;
			}
			else
			{
				DS_ASSERT(bvh->axisCount == 3);
				addBoxFunc = (AddBoxFunction)&dsAlignedBox3f_addBox;
				maxAxisFunc = &maxAxis3f;
			}
			compareFunc = &compareBoundsf;
			break;
		case dsGeometryElement_Double:
			if (bvh->axisCount == 2)
			{
				addBoxFunc = (AddBoxFunction)&dsAlignedBox2d_addBox;
				maxAxisFunc = &maxAxis2d;
			}
			else
			{
				DS_ASSERT(bvh->axisCount == 3);
				addBoxFunc = (AddBoxFunction)&dsAlignedBox3d_addBox;
				maxAxisFunc = &maxAxis3d;
			}
			compareFunc = &compareBoundsd;
			break;
		case dsGeometryElement_Int:
			if (bvh->axisCount == 2)
			{
				addBoxFunc = (AddBoxFunction)&dsAlignedBox2i_addBox;
				maxAxisFunc = &maxAxis2i;
			}
			else
			{
				DS_ASSERT(bvh->axisCount == 3);
				addBoxFunc = (AddBoxFunction)&dsAlignedBox3i_addBox;
				maxAxisFunc = &maxAxis3i;
			}
			compareFunc = &compareBoundsi;
			break;
		default:
			DS_ASSERT(false);
			return false;
	}

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

		for (size_t i = 0; i < objectCount; ++i)
		{
			dsBVHNode* node = getNode(bvh->tempNodes, bvh->nodeSize, i);
			node->object = getObject(objects, objectSize, i);
			if (!bvh->objectBoundsFunc(node->bounds, bvh, node->object))
				return false;

			node->leftNode = node->rightNode = INVALID_NODE;
		}

		rootNode = buildBVHBalancedRec(bvh, 0, objectCount, addBoxFunc, maxAxisFunc, compareFunc);
	}
	else
		rootNode = buildBVHRec(bvh, objects, 0, objectCount, objectSize, addBoxFunc);

	if (rootNode == INVALID_NODE)
	{
		dsBVH_clear(bvh);
		return false;
	}

	DS_ASSERT(rootNode == 0);
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

uint32_t dsBVH_intersect(const dsBVH* bvh, const void* bounds, dsBVHVisitFunction visitor,
	void* userData)
{
	if (!bvh || bvh->nodeCount == 0 || !bounds)
		return 0;

	IntersectFunction intersectFunc;
	switch (bvh->element)
	{
		case dsGeometryElement_Float:
			if (bvh->axisCount == 2)
				intersectFunc = (IntersectFunction)&dsAlignedBox2f_intersects;
			else
			{
				DS_ASSERT(bvh->axisCount == 3);
				intersectFunc = (IntersectFunction)&dsAlignedBox3f_intersects;
			}
			break;
		case dsGeometryElement_Double:
			if (bvh->axisCount == 2)
				intersectFunc = (IntersectFunction)&dsAlignedBox2d_intersects;
			else
			{
				DS_ASSERT(bvh->axisCount == 3);
				intersectFunc = (IntersectFunction)&dsAlignedBox3d_intersects;
			}
			break;
		case dsGeometryElement_Int:
			if (bvh->axisCount == 2)
				intersectFunc = (IntersectFunction)&dsAlignedBox2i_intersects;
			else
			{
				DS_ASSERT(bvh->axisCount == 3);
				intersectFunc = (IntersectFunction)&dsAlignedBox3i_intersects;
			}
			break;
		default:
			DS_ASSERT(false);
			return 0;
	}

	uint32_t count = 0;
	intersectBVHRec(bvh, &count, bvh->nodes, bounds, visitor, userData, intersectFunc);
	return count;
}

void dsBVH_clear(dsBVH* bvh)
{
	if (!bvh)
		return;

	bvh->nodeCount = 0;
	bvh->objectBoundsFunc = NULL;
}

void dsBVH_destroy(dsBVH* bvh)
{
	if (!bvh || !bvh->allocator)
		return;

	DS_VERIFY(dsAllocator_free(bvh->allocator, bvh->nodes));
	DS_VERIFY(dsAllocator_free(bvh->allocator, bvh->tempNodes));
	DS_VERIFY(dsAllocator_free(bvh->allocator, bvh));
}
