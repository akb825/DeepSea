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

#include "MaterialInfo.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/ResourceManager.h>
#include <DeepSea/Render/Renderer.h>
#include <DeepSea/Render/Types.h>
#include <string.h>

extern const char* dsResourceManager_noContextError;

struct dsShaderVariableGroup
{
	dsResourceManager* resourceManager;
	dsAllocator* allocator;
	const dsShaderVariableGroupDesc* description;
	dsGfxBuffer* buffer;
	uint8_t* rawData;
	dsShaderVariablePos* rawDataPositions;
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
		return pos->matrixColStride*dsMaterialType_matrixRows(element->type);
	}
	else
		return dsMaterialType_size(element->type);
}

static void memcpyData(void* result, dsMaterialType type, const dsShaderVariablePos* pos,
	const void* data, uint32_t count, bool isArray)
{
	uint32_t baseStride = dsMaterialType_size(type);
	uint32_t stride = isArray ? pos->stride : baseStride;
	DS_ASSERT(stride >= baseStride);

	// Check to see if the packing of the data matches.
	if (baseStride == stride)
		memcpy(result, data, stride*count);
	else if (type >= dsMaterialType_Mat2 && type <= dsMaterialType_DMat4x3)
	{
		// Matrices may have thier own stride.
		unsigned int rows = dsMaterialType_matrixRows(type);
		unsigned int columns = dsMaterialType_matrixColumns(type);
		uint32_t baseMatrixColStride = rows*dsMaterialType_machineAlignment(type);
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

static bool copyBuffer(dsCommandBuffer* commandBuffer, dsShaderVariableGroup* group,
	uint32_t elementIndex, const void* data, uint32_t firstIndex, uint32_t count)
{
	const dsShaderVariableElement* element = group->description->elements + elementIndex;
	const dsShaderVariablePos* pos = group->description->positions + elementIndex;
	uint32_t baseStride = dsMaterialType_size(element->type);
	uint32_t stride = element->count > 0 ? pos->stride : baseStride;
	DS_ASSERT(stride >= baseStride);

	if (stride == baseStride)
	{
		return dsGfxBuffer_copyData(commandBuffer, group->buffer, pos->offset + stride*firstIndex,
			data, count*stride);
	}
	else
	{
		// Need to use an intermediate buffer to resolve different packing.
		uint8_t staticBuffer[1024];
		uint8_t* buffer = staticBuffer;

		dsAllocator* allocator = NULL;
		uint32_t size = count*stride;
		if (size > sizeof(staticBuffer))
		{
			allocator = group->allocator;
			if (!allocator)
				allocator = group->resourceManager->allocator;
			if (!allocator)
			{
				errno = EPERM;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Couldn't find an allocator for temporary buffer.");
				return false;
			}

			buffer = (uint8_t*)dsAllocator_alloc(allocator, size);
			if (!buffer)
				return false;
		}

		memcpyData(buffer, element->type, pos, data, count, element->count > 0);
		bool result = dsGfxBuffer_copyData(commandBuffer, group->buffer, pos->offset +
			stride*firstIndex, buffer, count*stride);

		if (allocator)
			DS_VERIFY(dsAllocator_free(allocator, buffer));
		return result;
	}
}

bool dsShaderVariableGroup_useGfxBuffer(const dsResourceManager* resourceManager)
{
	if (!resourceManager)
		return false;

	return (resourceManager->supportedBuffers & dsGfxBufferUsage_UniformBlock) != 0;
}

dsShaderVariableGroup* dsShaderVariableGroup_create(dsResourceManager* resourceManager,
	dsAllocator* allocator, const dsShaderVariableGroupDesc* description)
{
	DS_PROFILE_FUNC_START();

	if (!resourceManager || (!allocator && !resourceManager->allocator) || !description ||
		!description->elements || description->elementCount == 0)
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

	dsShaderVariableGroup* group;
	if (dsShaderVariableGroup_useGfxBuffer(resourceManager))
	{
		if (!description->positions)
		{
			errno = EINVAL;
			DS_PROFILE_FUNC_RETURN(NULL);
		}

		uint32_t lastEleement = description->elementCount - 1;
		uint32_t size = description->positions[lastEleement].offset +
			elementSize(description->elements + lastEleement,
				description->positions + lastEleement);
		dsGfxBuffer* buffer = dsGfxBuffer_create(resourceManager, allocator,
			dsGfxBufferUsage_UniformBlock | dsGfxBufferUsage_CopyTo,
			dsGfxMemory_Draw | dsGfxMemory_Dynamic, NULL, size);
		if (!buffer)
			DS_PROFILE_FUNC_RETURN(NULL);

		group = (dsShaderVariableGroup*)dsAllocator_alloc(allocator, sizeof(dsShaderVariableGroup));
		if (!group)
		{
			DS_VERIFY(dsGfxBuffer_destroy(buffer));
			DS_PROFILE_FUNC_RETURN(NULL);
		}

		group->resourceManager = resourceManager;
		if (allocator->freeFunc)
			group->allocator = allocator;
		else
			group->allocator = NULL;
		group->description = description;
		group->buffer = buffer;
		group->rawData = NULL;
		group->rawDataPositions = NULL;
	}
	else
	{
		// Calculate full size ahead of time.
		uint32_t dataSize = 0;
		for (uint32_t i = 0; i < description->elementCount; ++i)
		{
			// Guarantee machine alignment.
			dsMaterialType type = description->elements[i].type;
			uint32_t alignment = dsMaterialType_machineAlignment(type);
			dataSize = ((dataSize + alignment - 1)/alignment)*alignment;

			uint32_t count = description->elements[i].count;
			if (count == 0)
				count = 1;
			dataSize += dsMaterialType_size(type)*count;
		}

		// Make a single allocation for the structure and its memebers.
		size_t totalSize = DS_ALIGNED_SIZE(sizeof(dsShaderVariableGroup)) +
			DS_ALIGNED_SIZE(dataSize) + sizeof(dsShaderVariablePos)*description->elementCount;

		void* fullMem = dsAllocator_alloc(allocator, totalSize);
		if (!fullMem)
			DS_PROFILE_FUNC_RETURN(NULL);

		dsBufferAllocator bufferAllocator;
		DS_VERIFY(dsBufferAllocator_initialize(&bufferAllocator, fullMem, totalSize));

		group = dsAllocator_alloc((dsAllocator*)&bufferAllocator, sizeof(dsShaderVariableGroup));
		DS_ASSERT(group);

		group->resourceManager = resourceManager;
		if (allocator->freeFunc)
			group->allocator = allocator;
		else
			group->allocator = NULL;
		group->description = description;
		group->buffer = NULL;
		group->rawData = dsAllocator_alloc((dsAllocator*)&bufferAllocator, dataSize);
		DS_ASSERT(group->rawData);
		group->rawDataPositions = (dsShaderVariablePos*)dsAllocator_alloc(
			(dsAllocator*)&bufferAllocator, sizeof(dsShaderVariablePos)*description->elementCount);
		DS_ASSERT(group->rawDataPositions);

		// Cache the position of each element.
		for (uint32_t i = 0, curOffset = 0; i < description->elementCount; ++i)
		{
			// Guarantee machine alignment.
			dsMaterialType type = description->elements[i].type;
			uint32_t alignment = dsMaterialType_machineAlignment(type);
			curOffset = ((curOffset + alignment - 1)/alignment)*alignment;

			uint32_t count = description->elements[i].count;
			if (count == 0)
				count = 1;
			uint16_t stride = dsMaterialType_size(type);

			group->rawDataPositions[i].offset = curOffset;
			group->rawDataPositions[i].stride = stride;
			group->rawDataPositions[i].matrixColStride =
				(uint16_t)(dsMaterialType_matrixRows(type)*dsMaterialType_machineAlignment(type));

			curOffset += stride*count;
		}
	}

	DS_ATOMIC_FETCH_ADD32(&resourceManager->shaderVariableGroupCount, 1);
	DS_PROFILE_FUNC_RETURN(group);
}

const dsShaderVariableGroupDesc* dsShaderVariableGroup_getDescription(
	const dsShaderVariableGroup* group)
{
	if (!group)
		return NULL;

	return group->description;
}

bool dsShaderVariableGroup_setElementData(dsCommandBuffer* commandBuffer,
	dsShaderVariableGroup* group, uint32_t element, const void* data, dsMaterialType type,
	uint32_t firstIndex, uint32_t count)
{
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !group || !data)
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
	if (firstIndex + count > maxCount)
	{
		errno = EINDEX;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Invalid shader variable group element.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (group->buffer)
	{
		bool success = copyBuffer(commandBuffer, group, element, data, firstIndex, count);
		DS_PROFILE_FUNC_RETURN(success);
	}
	else
	{
		// Stride is always set for raw data, even if not an array.
		uint32_t stride = group->rawDataPositions[element].stride;
		memcpy(group->rawData + group->rawDataPositions[element].offset + stride*firstIndex,
		data, stride*count);
		DS_PROFILE_FUNC_RETURN(true);
	}
}

dsGfxBuffer* dsShaderVariableGroup_getGfxBuffer(const dsShaderVariableGroup* group)
{
	if (!group)
		return NULL;

	return group->buffer;
}

const void* dsShaderVariableGroup_getElementData(const dsShaderVariableGroup* group,
	uint32_t element)
{
	if (!group || !group->rawData || element >= group->description->elementCount)
		return NULL;

	DS_ASSERT(group->rawDataPositions);
	return group->rawData + group->rawDataPositions[element].offset;
}

bool dsShaderVariableGorup_destroy(dsShaderVariableGroup* group)
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
