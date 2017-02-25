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

#include "MockShaderVariableGroupDesc.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <string.h>

dsShaderVariableGroupDesc* dsMockShaderVariableGroupDesc_create(dsResourceManager* resourceManager,
	dsAllocator* allocator, const dsShaderVariableElement* elements, uint32_t elementCount)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(allocator);
	DS_ASSERT(elements || elementCount == 0);

	size_t size = DS_ALIGNED_SIZE(sizeof(dsShaderVariableGroupDesc)) +
		DS_ALIGNED_SIZE(sizeof(dsShaderVariableElement)*elementCount);
	void* buffer = dsAllocator_alloc(allocator, size);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAllocator;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAllocator, buffer, size));

	dsShaderVariableGroupDesc* materialDesc = (dsShaderVariableGroupDesc*)dsAllocator_alloc(
		(dsAllocator*)&bufferAllocator, sizeof(dsShaderVariableGroupDesc));
	DS_ASSERT(materialDesc);

	materialDesc->resourceManager = resourceManager;
	if (allocator->freeFunc)
		materialDesc->allocator = allocator;
	else
		materialDesc->allocator = NULL;
	materialDesc->elementCount = elementCount;
	if (elementCount > 0)
	{
		materialDesc->elements = (dsShaderVariableElement*)dsAllocator_alloc(
			(dsAllocator*)&bufferAllocator, sizeof(dsShaderVariableElement)*elementCount);
		DS_ASSERT(materialDesc->elements);
		memcpy(materialDesc->elements, elements, sizeof(dsShaderVariableElement)*elementCount);
	}

	return materialDesc;
}

bool dsMockShaderVariableGroupDesc_destroy(dsResourceManager* resourceManager,
	dsShaderVariableGroupDesc* materialDesc)
{
	DS_UNUSED(resourceManager);
	if (materialDesc->allocator)
		return dsAllocator_free(materialDesc->allocator, materialDesc);
	return true;
}
