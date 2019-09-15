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

#include <DeepSea/Scene/ItemLists/SceneInstanceVariables.h>

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
#include <DeepSea/Scene/Types.h>

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

typedef struct dsSceneInstanceVariables
{
	dsSceneInstanceData instanceData;
	dsResourceManager* resourceManager;
	const dsShaderVariableGroupDesc* dataDesc;
	uint32_t nameID;
	uint32_t instanceSize;
	uint32_t stride;

	dsPopulateSceneInstanceVariablesFunction populateDataFunc;
	void* userData;
	dsDestroySceneUserDataFunction destroyUserDataFunc;

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
} dsSceneInstanceVariables;

static bool reserveSpace(dsSceneInstanceVariables* variables, uint32_t maxInstances)
{
	if (variables->curBufferData)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Attempting to populate scene instance variables before "
			"calling dsSceneInstanceData_finish() for the last usage.");
		return false;
	}

	if (maxInstances == 0)
		return true;

	dsAllocator* allocator = ((dsSceneInstanceData*)variables)->allocator;
	if (variables->fallback)
	{
		if (!variables->tempData || variables->maxTempInstances < maxInstances)
		{
			DS_VERIFY(dsAllocator_free(allocator, variables->tempData));
			variables->tempData = dsAllocator_alloc(allocator, maxInstances*variables->stride);
		}
		variables->curBufferData = variables->tempData;
		return variables->curBufferData != NULL;
	}

	size_t requiredSize = (size_t)variables->instanceSize*maxInstances;
	dsResourceManager* resourceManager = variables->resourceManager;
	uint64_t frameNumber = resourceManager->renderer->frameNumber;

	// Look for any buffer large enough that's FRAME_DELAY number of frames earlier than the
	// current one.
	for (uint32_t i = 0; i < variables->bufferCount;)
	{
		BufferInfo* bufferInfo = variables->buffers + i;
		if (bufferInfo->lastUsedFrame + FRAME_DELAY > frameNumber)
		{
			++i;
			continue;
		}

		if (bufferInfo->buffer->size >= requiredSize)
		{
			// Found
			bufferInfo->lastUsedFrame = frameNumber;
			variables->curBuffer = bufferInfo;
			break;
		}

		// This buffer is too small. Delete it now since a new one will need to be allocated.
		if (!dsGfxBuffer_destroy(bufferInfo->buffer))
			return false;

		// Constant-time removal since order doesn't matter.
		*bufferInfo = variables->buffers[variables->bufferCount - 1];
		--variables->bufferCount;
	}

	if (!variables->curBuffer)
	{
		// Not found: need to add a new buffer.
		uint32_t index = variables->bufferCount;
		if (!DS_RESIZEABLE_ARRAY_ADD(allocator, variables->buffers, variables->bufferCount,
				variables->maxBuffers, 1))
		{
			return false;
		}

		dsGfxMemory memoryHints = dsGfxMemory_Stream | dsGfxMemory_Synchronize;
		BufferInfo* bufferInfo = variables->buffers + index;
		bufferInfo->buffer = dsGfxBuffer_create(resourceManager, allocator,
			dsGfxBufferUsage_UniformBlock, memoryHints, NULL, requiredSize);
		if (!bufferInfo->buffer)
		{
			--variables->bufferCount;
			return false;
		}

		bufferInfo->lastUsedFrame = frameNumber;
		variables->curBuffer = bufferInfo;
	}

	DS_ASSERT(variables->curBuffer);
	variables->curBufferData = dsGfxBuffer_map(variables->curBuffer->buffer,
		dsGfxBufferMap_Write, 0, DS_MAP_FULL_BUFFER);
	return variables->curBufferData != NULL;
}

bool dsSceneInstanceVariables_populateData(dsSceneInstanceData* instanceData,
	const dsView* view, const dsSceneInstanceInfo* instances, uint32_t instanceCount)
{
	dsSceneInstanceVariables* variables = (dsSceneInstanceVariables*)instanceData;
	DS_ASSERT(variables);
	DS_ASSERT(view);

	variables->curInstance = 0;
	variables->curInstanceCount = 0;
	if (!reserveSpace(variables, instanceCount))
		return false;

	variables->curInstanceCount = instanceCount;
	variables->populateDataFunc(variables->userData, view, instances, instanceCount,
		variables->curBufferData, variables->stride);
	BufferInfo* curBuffer = variables->curBuffer;
	if (curBuffer)
		DS_VERIFY(dsGfxBuffer_unmap(curBuffer->buffer));

	return true;
}

bool dsSceneInstanceVariables_bindInstance(dsSceneInstanceData* instanceData, uint32_t index,
	dsSharedMaterialValues* values)
{
	dsSceneInstanceVariables* variables = (dsSceneInstanceVariables*)instanceData;
	DS_ASSERT(variables);
	DS_ASSERT(values);

	if (index >= variables->curInstanceCount)
	{
		errno = EINDEX;
		return false;
	}

	BufferInfo* curBuffer = variables->curBuffer;
	if (curBuffer)
	{
		return dsSharedMaterialValues_setBufferID(values, variables->nameID, curBuffer->buffer,
			index*variables->stride, variables->instanceSize);
	}

	// Instance count should be 0 if not in a valid state, so should only get here if fallback.
	DS_ASSERT(variables->fallback);
	DS_ASSERT(variables->tempData);

	// Need to copy the data to the ShaderVariableGroup instance.
	const dsShaderVariableGroupDesc* dataDesc = variables->dataDesc;
	dsShaderVariableGroup* group = variables->fallback;
	const uint8_t* tempData = variables->tempData + index*variables->stride;
	uint8_t packedData[sizeof(dsMatrix44d)];
	for (uint32_t i = 0; i < dataDesc->elementCount; ++i)
	{
		const dsShaderVariableElement* element = dataDesc->elements + i;
		const dsShaderVariablePos* pos = dataDesc->positions + i;
		const CPUInfo* cpuInfo = variables->cpuInfo + i;
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

bool dsSceneInstanceVariables_finish(dsSceneInstanceData* instanceData)
{
	dsSceneInstanceVariables* variables = (dsSceneInstanceVariables*)instanceData;
	DS_ASSERT(variables);

	variables->curBuffer = NULL;
	variables->curBufferData = NULL;
	variables->curInstanceCount = 0;
	return true;
}

bool dsSceneInstanceVariables_destroy(dsSceneInstanceData* instanceData)
{
	dsSceneInstanceVariables* variables = (dsSceneInstanceVariables*)instanceData;
	DS_ASSERT(variables);

	for (uint32_t i = 0; i < variables->bufferCount; ++i)
	{
		if (!dsGfxBuffer_destroy(variables->buffers[i].buffer))
		{
			DS_ASSERT(i == 0);
			return false;
		}
	}

	if (!dsShaderVariableGroup_destroy(variables->fallback))
		return false;

	if (variables->destroyUserDataFunc)
		variables->destroyUserDataFunc(variables->userData);

	DS_VERIFY(dsAllocator_free(instanceData->allocator, variables->buffers));
	DS_VERIFY(dsAllocator_free(instanceData->allocator, variables->tempData));
	DS_VERIFY(dsAllocator_free(instanceData->allocator, variables));
	return true;
}

dsSceneInstanceData* dsSceneInstanceVariables_create(dsAllocator* allocator,
	dsResourceManager* resourceManager, const dsShaderVariableGroupDesc* dataDesc, uint32_t nameID,
	dsPopulateSceneInstanceVariablesFunction populateDataFunc, void* userData,
	dsDestroySceneUserDataFunction destroyUserDataFunc)
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

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSceneInstanceVariables));
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

	dsSceneInstanceVariables* variables = DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneInstanceVariables);
	DS_ASSERT(variables);
	dsSceneInstanceData* instanceData = (dsSceneInstanceData*)variables;
	instanceData->allocator = allocator;
	instanceData->valueCount = 1;
	instanceData->populateDataFunc = &dsSceneInstanceVariables_populateData;
	instanceData->bindInstanceFunc = &dsSceneInstanceVariables_bindInstance;
	instanceData->finishFunc = &dsSceneInstanceVariables_finish;
	instanceData->destroyFunc = &dsSceneInstanceVariables_destroy;

	variables->resourceManager = resourceManager;
	variables->dataDesc = dataDesc;
	variables->nameID = nameID;

	size_t instanceSize = 0;
	for (uint32_t i = 0; i < dataDesc->elementCount; ++i)
	{
		const dsShaderVariableElement* element = dataDesc->elements + i;
		dsMaterialType_addElementBlockSize(&instanceSize, element->type, element->count);
	}
	DS_ASSERT(instanceSize <= UINT_MAX);
	variables->instanceSize = (uint32_t)instanceSize;

	size_t stride = instanceSize;
	if (resourceManager->minUniformBlockAlignment > 0)
		stride = DS_CUSTOM_ALIGNED_SIZE(stride, resourceManager->minUniformBlockAlignment);
	DS_ASSERT(stride <= UINT_MAX);
	variables->stride = (uint32_t)stride;

	variables->populateDataFunc = populateDataFunc;
	variables->userData = userData;
	variables->destroyUserDataFunc = destroyUserDataFunc;

	variables->buffers = NULL;
	variables->bufferCount = 0;
	variables->maxBuffers = 0;

	if (needsFallback)
	{
		variables->fallback = dsShaderVariableGroup_create(resourceManager,
			(dsAllocator*)&bufferAlloc, NULL, dataDesc);
		DS_ASSERT(variables->fallback);

		variables->cpuInfo = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, CPUInfo,
			dataDesc->elementCount);
		DS_ASSERT(variables->cpuInfo);
		for (uint32_t i = 0; i < dataDesc->elementCount; ++i)
		{
			const dsShaderVariableElement* element = dataDesc->elements + i;
			variables->cpuInfo[i].elementSize = dsMaterialType_cpuSize(element->type);
			variables->cpuInfo[i].columnCount = dsMaterialType_matrixColumns(element->type);
			variables->cpuInfo[i].columnSize =
				dsMaterialType_cpuSize(dsMaterialType_matrixColumnType(element->type));
			DS_ASSERT(variables->cpuInfo[i].elementSize > 0);
		}
	}
	else
	{
		variables->fallback = NULL;
		variables->cpuInfo = NULL;
	}

	variables->tempData = NULL;
	variables->maxTempInstances = 0;

	variables->curBuffer = NULL;
	variables->curBufferData = NULL;
	variables->curInstance = 0;
	variables->curInstanceCount = 0;

	return instanceData;
}
