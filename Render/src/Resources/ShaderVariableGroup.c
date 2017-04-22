/*
 * Copyright 2017 Aaron Barany
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

#include <DeepSea/Render/Resources/ShaderVariableGroup.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/MaterialType.h>
#include <DeepSea/Render/Resources/ResourceManager.h>
#include <DeepSea/Render/Types.h>
#include <string.h>

extern const char* dsResourceManager_noContextError;

typedef struct PositionInfo
{
	dsShaderVariablePos pos;
	bool dirty;
} PositionInfo;

struct dsShaderVariableGroup
{
	dsResourceManager* resourceManager;
	dsAllocator* allocator;
	const dsShaderVariableGroupDesc* description;
	dsGfxBuffer* buffer;
	uint8_t* rawData;
	PositionInfo* rawDataPositions;
	size_t dirtyStart;
	size_t dirtyEnd;
};

static uint32_t elementSize(const dsShaderVariableElement* element, const dsShaderVariablePos* pos)
{
	if (element->count > 0)
	{
		DS_ASSERT(pos->stride > 0);
		return element->count*pos->stride;
	}
	else if (element->type >= dsMaterialType_Mat2 && element->type <= dsMaterialType_DMat4x3)
	{
		DS_ASSERT(pos->matrixColStride > 0);
		return pos->matrixColStride*dsMaterialType_matrixColumns(element->type);
	}
	else
		return dsMaterialType_cpuSize(element->type);
}

static size_t getRawBufferSize(const dsShaderVariableGroupDesc* description, bool useGfxBuffer)
{
	if (useGfxBuffer)
	{
		uint32_t lastEleement = description->elementCount - 1;
		return description->positions[lastEleement].offset +
			elementSize(description->elements + lastEleement,
				description->positions + lastEleement);
	}

	size_t dataSize = 0;
	for (uint32_t i = 0; i < description->elementCount; ++i)
	{
		dsMaterialType_addElementCpuSize(&dataSize, description->elements[i].type,
			description->elements[i].count);
	}
	return dataSize;
}

static void memcpyData(void* result, const dsShaderVariableElement* element,
	const dsShaderVariablePos* pos, const void* data, uint32_t count)
{
	uint32_t baseStride = dsMaterialType_cpuSize(element->type);
	uint32_t stride = element->count > 0 ? pos->stride : elementSize(element, pos);
	DS_ASSERT(stride >= baseStride);

	// Check to see if the packing of the data matches.
	if (baseStride == stride)
		memcpy(result, data, stride*count);
	else if (element->type >= dsMaterialType_Mat2 && element->type <= dsMaterialType_DMat4x3)
	{
		// Matrices may have thier own stride.
		unsigned int columns = dsMaterialType_matrixColumns(element->type);
		uint32_t baseMatrixColStride = dsMaterialType_cpuSize(
			dsMaterialType_matrixColumnType(element->type));
		uint32_t matrixColStride = pos->matrixColStride;
		DS_ASSERT(matrixColStride >= baseMatrixColStride);
		DS_ASSERT(baseMatrixColStride*columns == baseStride);
		DS_ASSERT(matrixColStride*columns == stride);

		uint8_t* curResult = (uint8_t*)result;
		const uint8_t* curData = (const uint8_t*)data;
		uint32_t totalColumns = count*columns;
		for (uint32_t i = 0; i < totalColumns; ++i, curResult += matrixColStride,
			curData += baseMatrixColStride)
		{
			memcpy(curResult, curData, baseMatrixColStride);
		}
	}
	else
	{
		uint8_t* curResult = (uint8_t*)result;
		const uint8_t* curData = (const uint8_t*)data;
		for (uint32_t i = 0; i < count; ++i, curResult += stride, curData += baseStride)
			memcpy(curResult, curData, baseStride);
	}
}

size_t dsShaderVariableGroup_sizeof(void)
{
	return sizeof(dsShaderVariableGroup);
}

size_t dsShaderVariableGroup_fullAllocSize(
	const dsResourceManager* resourceManager, const dsShaderVariableGroupDesc* description)
{
	if (!resourceManager || !description)
		return 0;

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsShaderVariableGroup));

	bool useGfxBuffer = dsShaderVariableGroup_useGfxBuffer(resourceManager);
	fullSize += DS_ALIGNED_SIZE(getRawBufferSize(description, useGfxBuffer));
	if (!useGfxBuffer)
		fullSize += DS_ALIGNED_SIZE(description->elementCount*sizeof(PositionInfo));

	return fullSize;
}

bool dsShaderVariableGroup_useGfxBuffer(const dsResourceManager* resourceManager)
{
	if (!resourceManager)
		return false;

	return (resourceManager->supportedBuffers & dsGfxBufferUsage_UniformBlock) != 0;
}

dsShaderVariableGroup* dsShaderVariableGroup_create(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsAllocator* gfxBufferAllocator,
	const dsShaderVariableGroupDesc* description)
{
	DS_PROFILE_FUNC_START();

	if (!resourceManager || (!allocator && !resourceManager->allocator) ||
		(!gfxBufferAllocator && !resourceManager->allocator) || !description)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, dsResourceManager_noContextError);
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!allocator)
		allocator = resourceManager->allocator;

	bool useGfxBuffer = dsShaderVariableGroup_useGfxBuffer(resourceManager);
	if (useGfxBuffer && !description->positions)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	size_t totalSize = dsShaderVariableGroup_fullAllocSize(resourceManager, description);
	DS_ASSERT(totalSize > 0);
	void* fullMem = dsAllocator_alloc(allocator, totalSize);
	if (!fullMem)
		DS_PROFILE_FUNC_RETURN(NULL);

	dsBufferAllocator bufferAllocator;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAllocator, fullMem, totalSize));

	dsShaderVariableGroup* group = dsAllocator_alloc((dsAllocator*)&bufferAllocator,
		sizeof(dsShaderVariableGroup));
	DS_ASSERT(group);

	group->resourceManager = resourceManager;
	group->allocator = dsAllocator_keepPointer(allocator);
	group->description = description;
	group->buffer = NULL;
	group->rawData = NULL;
	group->rawDataPositions = NULL;
	group->dirtyStart = (size_t)-1;
	group->dirtyEnd = 0;

	size_t bufferSize = getRawBufferSize(description, useGfxBuffer);
	DS_ASSERT(bufferSize > 0);
	if (useGfxBuffer)
	{
		group->buffer = dsGfxBuffer_create(resourceManager, gfxBufferAllocator,
			dsGfxBufferUsage_UniformBlock | dsGfxBufferUsage_CopyFrom | dsGfxBufferUsage_CopyTo,
			dsGfxMemory_Draw | dsGfxMemory_Dynamic | dsGfxMemory_Read, NULL,
			DS_ALIGNED_SIZE(bufferSize));

		if (!group->buffer)
		{
			dsAllocator_free(allocator, group);
			DS_PROFILE_FUNC_RETURN(NULL);
		}
	}

	group->rawData = dsAllocator_alloc((dsAllocator*)&bufferAllocator, bufferSize);
	DS_ASSERT(group->rawData);

	// Cache the position of each element.
	if (!useGfxBuffer)
	{
		group->rawDataPositions = (PositionInfo*)dsAllocator_alloc(
			(dsAllocator*)&bufferAllocator, sizeof(PositionInfo)*description->elementCount);
		DS_ASSERT(group->rawDataPositions);

		size_t curSize = 0;
		for (uint32_t i = 0; i < description->elementCount; ++i)
		{
			dsMaterialType type = description->elements[i].type;
			uint16_t stride = dsMaterialType_cpuSize(type);

			group->rawDataPositions[i].pos.offset = (uint32_t)dsMaterialType_addElementCpuSize(
				&curSize, type, description->elements[i].count);
			group->rawDataPositions[i].pos.stride = stride;
			group->rawDataPositions[i].pos.matrixColStride = dsMaterialType_cpuSize(
				dsMaterialType_matrixColumnType(type));
			group->rawDataPositions[i].dirty = false;
		}
	}

	DS_ATOMIC_FETCH_ADD32(&resourceManager->shaderVariableGroupCount, 1);
	DS_PROFILE_FUNC_RETURN(group);
}

const dsShaderVariableGroupDesc* dsShaderVariableGroup_getDescription(
	const dsShaderVariableGroup* group)
{
	if (!group)
	{
		errno = EINVAL;
		return NULL;
	}

	return group->description;
}

bool dsShaderVariableGroup_setElementData(dsShaderVariableGroup* group, uint32_t element,
	const void* data, dsMaterialType type, uint32_t firstIndex, uint32_t count)
{
	DS_PROFILE_FUNC_START();

	if (!group || !data || count == 0)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (element >= group->description->elementCount)
	{
		errno = EINDEX;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Invalid shader variable group element.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (type != group->description->elements[element].type)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Type doesn't match shader variable group element type.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	uint32_t maxCount = group->description->elements[element].count;
	if (maxCount == 0)
		maxCount = 1;
	if (!DS_IS_BUFFER_RANGE_VALID(firstIndex, count, maxCount))
	{
		errno = EINDEX;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to copy too many elements.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	const dsShaderVariablePos* pos;
	uint32_t stride;
	if (group->rawDataPositions)
	{
		pos = &group->rawDataPositions[element].pos;

		// Stride is always set for raw data, even if not an array.
		stride = pos->stride;
		group->rawDataPositions[element].dirty = true;
	}
	else
	{
		DS_ASSERT(group->description->positions);
		pos = group->description->positions + element;

		if (group->description->elements[element].count == 0)
			stride = dsMaterialType_cpuSize(type);
		else
			stride = pos->stride;

		size_t start = pos->offset + stride*firstIndex;
		size_t end = start + count*stride;
		group->dirtyStart = dsMin(start, group->dirtyStart);
		group->dirtyEnd = dsMax(end, group->dirtyEnd);
	}
	memcpyData(group->rawData + pos->offset + stride*firstIndex,
		group->description->elements + element, pos, data, count);
	DS_PROFILE_FUNC_RETURN(true);
}

dsGfxBuffer* dsShaderVariableGroup_getGfxBuffer(const dsShaderVariableGroup* group)
{
	if (!group)
		return NULL;

	return group->buffer;
}

const void* dsShaderVariableGroup_getRawElementData(const dsShaderVariableGroup* group,
	uint32_t element)
{
	if (!group || !group->rawDataPositions || element >= group->description->elementCount)
		return NULL;

	DS_ASSERT(group->rawData);
	return group->rawData + group->rawDataPositions[element].pos.offset;
}

bool dsShaderVariableGroup_isElementDirty(const dsShaderVariableGroup* group,
	uint32_t element)
{
	if (!group || !group->rawDataPositions || element >= group->description->elementCount)
		return false;

	return group->rawDataPositions[element].dirty;
}

bool dsShaderVariableGroup_commit(dsCommandBuffer* commandBuffer, dsShaderVariableGroup* group)
{
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !group)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = true;
	if (group->buffer && group->dirtyStart < group->dirtyEnd)
	{
		success = dsGfxBuffer_copyData(commandBuffer, group->buffer, group->dirtyStart,
			group->rawData + group->dirtyStart, group->dirtyEnd - group->dirtyStart);
		if (success)
		{
			group->dirtyStart = (size_t)-1;
			group->dirtyEnd = 0;
		}
	}
	else if (group->rawDataPositions)
	{
		for (uint32_t i = 0; i < group->description->elementCount; ++i)
			group->rawDataPositions[i].dirty = false;
	}

	DS_PROFILE_FUNC_RETURN(success);
}

bool dsShaderVariableGroup_destroy(dsShaderVariableGroup* group)
{
	DS_PROFILE_FUNC_START();

	if (!group)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = group->resourceManager;
	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, dsResourceManager_noContextError);
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (group->buffer)
	{
		if (!dsGfxBuffer_destroy(group->buffer))
			DS_PROFILE_FUNC_RETURN(false);
	}

	if (group->allocator)
		DS_VERIFY(dsAllocator_free(group->allocator, group));

	DS_ATOMIC_FETCH_ADD32(&resourceManager->shaderVariableGroupCount, -1);
	DS_PROFILE_FUNC_RETURN(true);
}
