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

#include "Resources/MockShaderVariableGroupDesc.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Render/Resources/MaterialType.h>
#include <DeepSea/Render/Resources/ShaderVariableGroup.h>
#include <string.h>

dsShaderVariableGroupDesc* dsMockShaderVariableGroupDesc_create(dsResourceManager* resourceManager,
	dsAllocator* allocator, const dsShaderVariableElement* elements, uint32_t elementCount)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(allocator);
	DS_ASSERT(elements);
	DS_ASSERT(elementCount > 0);

	bool useGfxBuffer = dsShaderVariableGroup_useGfxBuffer(resourceManager);
	size_t size = DS_ALIGNED_SIZE(sizeof(dsShaderVariableGroupDesc)) +
		DS_ALIGNED_SIZE(sizeof(dsShaderVariableElement)*elementCount);
	if (useGfxBuffer)
		size += DS_ALIGNED_SIZE(sizeof(dsShaderVariablePos)*elementCount);
	void* buffer = dsAllocator_alloc(allocator, size);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAllocator;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAllocator, buffer, size));

	dsShaderVariableGroupDesc* groupDesc = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAllocator,
		dsShaderVariableGroupDesc);
	DS_ASSERT(groupDesc);

	groupDesc->resourceManager = resourceManager;
	groupDesc->allocator = dsAllocator_keepPointer(allocator);
	groupDesc->elementCount = elementCount;
	groupDesc->elements = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAllocator,
		dsShaderVariableElement, elementCount);
	DS_ASSERT(groupDesc->elements);
	memcpy(groupDesc->elements, elements, sizeof(dsShaderVariableElement)*elementCount);

	if (useGfxBuffer)
	{
		groupDesc->positions = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAllocator,
			dsShaderVariablePos, elementCount);
		DS_ASSERT(groupDesc->positions);
		size_t curSize = 0;
		for (uint32_t i = 0; i < elementCount; ++i)
		{
			groupDesc->positions[i].offset = (uint32_t)dsMaterialType_addElementBlockSize(&curSize,
				elements[i].type, elements[i].count);
			if (elements[i].count > 0)
				groupDesc->positions[i].stride = dsMaterialType_blockSize(elements[i].type, true);
			else
				groupDesc->positions[i].stride = 0;
			groupDesc->positions[i].matrixColStride = dsMaterialType_blockAlignment(
				dsMaterialType_matrixColumnType(elements[i].type), true);
		}
	}
	else
		groupDesc->positions = NULL;

	return groupDesc;
}

bool dsMockShaderVariableGroupDesc_destroy(dsResourceManager* resourceManager,
	dsShaderVariableGroupDesc* groupDesc)
{
	DS_UNUSED(resourceManager);
	if (groupDesc->allocator)
		return dsAllocator_free(groupDesc->allocator, groupDesc);
	return true;
}
