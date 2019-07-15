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

#include <DeepSea/Scene/SceneInstanceData.h>

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/MaterialType.h>
#include <DeepSea/Render/Resources/ShaderVariableGroup.h>
#include <DeepSea/Render/Resources/SharedMaterialValues.h>

#include <limits.h>
#include <string.h>

#define FRAME_DELAY 3

typedef struct BufferInfo
{
	dsGfxBuffer* buffer;
	uint64_t lastUsedFrame;
} BufferInfo;

typedef struct CPUInfo
{
	uint8_t elementSize;
	uint8_t columnCount;
	uint8_t columnSize;
} CPUInfo;

struct dsSceneInstanceData
{
	dsAllocator* allocator;
	dsResourceManager* resourceManager;
	const dsShaderVariableGroupDesc* dataDesc;
	uint32_t nameID;
	uint32_t instanceSize;
	uint32_t stride;

	dsPopulateSceneInstanceDataFunction populateDataFunc;
	void* userData;
	dsDestroySceneInstanceUserDataFunction destroyUserDataFunc;

	BufferInfo* buffers;
	uint32_t bufferCount;
	uint32_t maxBuffers;

	dsShaderVariableGroup* fallback;
	CPUInfo* cpuInfo;
	uint8_t* tempData;
	uint32_t maxTempInstances;

	BufferInfo* curBuffer;
	uint8_t* curBufferData;
	uint32_t curInstance;
	uint32_t curInstanceCount;
};

static bool reserveSpace(dsSceneInstanceData* instanceData, uint32_t maxInstances)
{
	if (instanceData->curBufferData)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Attempting to populate scene instance data before calling "
			"dsSceneInstanceData_finish() for the last usage.");
		return false;
	}

	if (maxInstances == 0)
		return true;

	if (instanceData->fallback)
	{
		if (!instanceData->tempData || instanceData->maxTempInstances < maxInstances)
		{
			DS_VERIFY(dsAllocator_free(instanceData->allocator, instanceData->tempData));
			instanceData->tempData = dsAllocator_alloc(instanceData->allocator,
				maxInstances*instanceData->stride);
		}
		instanceData->curBufferData = instanceData->tempData;
		return instanceData->curBufferData != NULL;
	}

	size_t requiredSize = (size_t)instanceData->instanceSize*maxInstances;
	dsResourceManager* resourceManager = instanceData->resourceManager;
	uint64_t frameNumber = resourceManager->renderer->frameNumber;

	// Look for any buffer large enough that's FRAME_DELAY number of frames earlier than the
	// current one.
	for (uint32_t i = 0; i < instanceData->bufferCount;)
	{
		BufferInfo* bufferInfo = instanceData->buffers + i;
		if (bufferInfo->lastUsedFrame + FRAME_DELAY > frameNumber)
		{
			++i;
			continue;
		}

		if (bufferInfo->buffer->size >= requiredSize)
		{
			// Found
			bufferInfo->lastUsedFrame = frameNumber;
			instanceData->curBuffer = bufferInfo;
			break;
		}

		// This buffer is too small. Delete it now since a new one will need to be allocated.
		if (!dsGfxBuffer_destroy(bufferInfo->buffer))
			return false;

		// Constant-time removal since order doesn't matter.
		*bufferInfo = instanceData->buffers[instanceData->bufferCount - 1];
		--instanceData->bufferCount;
	}

	if (!instanceData->curBuffer)
	{
		// Not found: need to add a new buffer.
		uint32_t index = instanceData->bufferCount;
		if (!DS_RESIZEABLE_ARRAY_ADD(instanceData->allocator, instanceData->buffers,
				instanceData->bufferCount, instanceData->maxBuffers, 1))
		{
			return false;
		}

		dsGfxMemory memoryHints = dsGfxMemory_Stream | dsGfxMemory_Synchronize;
		BufferInfo* bufferInfo = instanceData->buffers + index;
		bufferInfo->buffer = dsGfxBuffer_create(resourceManager, instanceData->allocator,
			dsGfxBufferUsage_UniformBlock, memoryHints, NULL, requiredSize);
		if (!bufferInfo->buffer)
		{
			--instanceData->bufferCount;
			return false;
		}

		bufferInfo->lastUsedFrame = frameNumber;
		instanceData->curBuffer = bufferInfo;
	}

	DS_ASSERT(instanceData->curBuffer);
	instanceData->curBufferData = dsGfxBuffer_map(instanceData->curBuffer->buffer,
		dsGfxBufferMap_Write, 0, DS_MAP_FULL_BUFFER);
	return instanceData->curBufferData != NULL;
}

dsSceneInstanceData* dsSceneInstanceData_create(dsAllocator* allocator,
	dsResourceManager* resourceManager, const dsShaderVariableGroupDesc* dataDesc, uint32_t nameID,
	dsPopulateSceneInstanceDataFunction populateDataFunc, void* userData,
	dsDestroySceneInstanceUserDataFunction destroyUserDataFunc)
{
	if (!allocator || !resourceManager || !dataDesc || !populateDataFunc)
	{
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG,
			"Scene instance data allocator must support freeing memory.");
		return NULL;
	}

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSceneInstanceData));
	bool needsFallback = !dsShaderVariableGroup_useGfxBuffer(resourceManager);
	if (needsFallback)
	{
		fullSize += DS_ALIGNED_SIZE(sizeof(CPUInfo)*dataDesc->elementCount) +
			dsShaderVariableGroup_fullAllocSize(resourceManager, dataDesc);
	}
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
	{
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		return NULL;
	}

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsSceneInstanceData* instanceData = DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneInstanceData);
	DS_ASSERT(instanceData);
	instanceData->allocator = allocator;
	instanceData->resourceManager = resourceManager;
	instanceData->dataDesc = dataDesc;
	instanceData->nameID = nameID;

	size_t instanceSize = 0;
	for (uint32_t i = 0; i < dataDesc->elementCount; ++i)
	{
		const dsShaderVariableElement* element = dataDesc->elements + i;
		dsMaterialType_addElementBlockSize(&instanceSize, element->type, element->count);
	}
	DS_ASSERT(instanceSize <= UINT_MAX);
	instanceData->instanceSize = (uint32_t)instanceSize;

	size_t stride = instanceSize;
	if (resourceManager->minUniformBlockAlignment > 0)
		stride = DS_CUSTOM_ALIGNED_SIZE(stride, resourceManager->minUniformBlockAlignment);
	DS_ASSERT(stride <= UINT_MAX);
	instanceData->stride = (uint32_t)stride;

	instanceData->populateDataFunc = populateDataFunc;
	instanceData->userData = userData;
	instanceData->destroyUserDataFunc = destroyUserDataFunc;

	instanceData->buffers = NULL;
	instanceData->bufferCount = 0;
	instanceData->maxBuffers = 0;

	if (needsFallback)
	{
		instanceData->fallback = dsShaderVariableGroup_create(resourceManager,
			(dsAllocator*)&bufferAlloc, NULL, dataDesc);
		DS_ASSERT(instanceData->fallback);

		instanceData->cpuInfo = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, CPUInfo,
			dataDesc->elementCount);
		DS_ASSERT(instanceData->cpuInfo);
		for (uint32_t i = 0; i < dataDesc->elementCount; ++i)
		{
			const dsShaderVariableElement* element = dataDesc->elements + i;
			instanceData->cpuInfo[i].elementSize = dsMaterialType_cpuSize(element->type);
			instanceData->cpuInfo[i].columnCount = dsMaterialType_matrixColumns(element->type);
			instanceData->cpuInfo[i].columnSize =
				dsMaterialType_cpuSize(dsMaterialType_matrixColumnType(element->type));
			DS_ASSERT(instanceData->cpuInfo[i].elementSize > 0);
		}
	}
	else
	{
		instanceData->fallback = NULL;
		instanceData->cpuInfo = NULL;
	}

	instanceData->tempData = NULL;
	instanceData->maxTempInstances = 0;

	instanceData->curBuffer = NULL;
	instanceData->curBufferData = NULL;
	instanceData->curInstance = 0;
	instanceData->curInstanceCount = 0;

	return instanceData;
}

bool dsSceneInstanceData_populateData(dsSceneInstanceData* instanceData,
	const dsView* view, const dsSceneInstanceInfo* instances, uint32_t instanceCount)
{
	if (instanceData)
	{
		instanceData->curInstance = 0;
		instanceData->curInstanceCount = 0;
	}

	if (!instanceData || !view || (instanceCount > 0 && !instances))
	{
		errno = EINVAL;
		return false;
	}

	if (!reserveSpace(instanceData, instanceCount))
		return false;

	instanceData->curInstanceCount = instanceCount;
	instanceData->populateDataFunc(instanceData->userData, view, instances, instanceCount,
		instanceData->curBufferData, instanceData->stride);
	BufferInfo* curBuffer = instanceData->curBuffer;
	if (curBuffer)
		DS_VERIFY(dsGfxBuffer_unmap(curBuffer->buffer));

	return true;
}

bool dsSceneInstanceData_bindInstance(const dsSceneInstanceData* instanceData,
	uint32_t index, dsSharedMaterialValues* values)
{
	if (!instanceData || !values)
	{
		errno = EINVAL;
		return false;
	}

	if (index >= instanceData->curInstanceCount)
	{
		errno = EINDEX;
		return false;
	}

	BufferInfo* curBuffer = instanceData->curBuffer;
	if (curBuffer)
	{
		return dsSharedMaterialValues_setBufferId(values, instanceData->nameID, curBuffer->buffer,
			index*instanceData->stride, instanceData->instanceSize);
	}

	// Instance count should be 0 if not in a valid state, so should only get here if fallback.
	DS_ASSERT(instanceData->fallback);
	DS_ASSERT(instanceData->tempData);

	// Need to copy the data to the ShaderVariableGroup instance.
	const dsShaderVariableGroupDesc* dataDesc = instanceData->dataDesc;
	dsShaderVariableGroup* group = instanceData->fallback;
	const uint8_t* tempData = instanceData->tempData + index*instanceData->stride;
	uint8_t packedData[sizeof(dsMatrix44d)];
	for (uint32_t i = 0; i < dataDesc->elementCount; ++i)
	{
		const dsShaderVariableElement* element = dataDesc->elements + i;
		const dsShaderVariablePos* pos = dataDesc->positions + i;
		const CPUInfo* cpuInfo = instanceData->cpuInfo + i;
		const uint8_t* curData = tempData + pos->offset;
		uint32_t count = dsMax(element->count, 1U);
		if (pos->matrixColStride > 0)
		{
			DS_ASSERT(cpuInfo->columnCount > 0);
			DS_ASSERT(cpuInfo->columnSize > 0);
			DS_ASSERT(pos->stride == pos->matrixColStride*cpuInfo->columnCount);
			for (uint32_t j = 0; j < count; ++j)
			{
				uint8_t* curCol = packedData;
				for (uint8_t k = 0; k < cpuInfo->columnCount; ++k, curCol += cpuInfo->columnSize,
					curData += pos->matrixColStride)
				{
					memcpy(curCol, curData, cpuInfo->columnSize);
				}
				DS_VERIFY(dsShaderVariableGroup_setElementData(
					group, i, packedData, element->type, j, 1));
			}
		}
		else
		{
			for (uint32_t j = 0; j < count; ++j, curData += pos->stride)
			{
				DS_VERIFY(dsShaderVariableGroup_setElementData(
					group, i, curData, element->type, j, 1));
			}
		}
	}

	DS_VERIFY(dsShaderVariableGroup_commitWithoutBuffer(group));
	return true;
}

bool dsSceneInstanceData_finish(dsSceneInstanceData* instanceData)
{
	if (!instanceData)
	{
		errno = EINVAL;
		return false;
	}

	instanceData->curBuffer = NULL;
	instanceData->curBufferData = NULL;
	instanceData->curInstanceCount = 0;
	return true;
}

bool dsSceneInstanceData_destroy(dsSceneInstanceData* instanceData)
{
	if (!instanceData)
		return true;

	for (uint32_t i = 0; i < instanceData->bufferCount; ++i)
	{
		if (!dsGfxBuffer_destroy(instanceData->buffers[i].buffer))
		{
			DS_ASSERT(i == 0);
			return false;
		}
	}

	if (!dsShaderVariableGroup_destroy(instanceData->fallback))
		return false;

	if (instanceData->destroyUserDataFunc)
		instanceData->destroyUserDataFunc(instanceData->userData);

	DS_VERIFY(dsAllocator_free(instanceData->allocator, instanceData->buffers));
	DS_VERIFY(dsAllocator_free(instanceData->allocator, instanceData->tempData));
	DS_VERIFY(dsAllocator_free(instanceData->allocator, instanceData));
	return true;
}
