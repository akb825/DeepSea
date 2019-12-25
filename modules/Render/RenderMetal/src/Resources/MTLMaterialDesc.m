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

#include "Resources/MTLMaterialDesc.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <string.h>

dsMaterialDesc* dsMTLMaterialDesc_create(dsResourceManager* resourceManager,
	dsAllocator* allocator, const dsMaterialElement* elements, uint32_t elementCount)
{
	size_t size = DS_ALIGNED_SIZE(sizeof(dsMaterialDesc)) +
		DS_ALIGNED_SIZE(sizeof(dsMaterialElement)*elementCount);
	for (uint32_t i = 0; i < elementCount; ++i)
		size += DS_ALIGNED_SIZE(strlen(elements[i].name) + 1);
	void* buffer = dsAllocator_alloc(allocator, size);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, size));

	dsMaterialDesc* materialDesc = DS_ALLOCATE_OBJECT(&bufferAlloc, dsMaterialDesc);
	DS_ASSERT(materialDesc);

	materialDesc->resourceManager = resourceManager;
	materialDesc->allocator = dsAllocator_keepPointer(allocator);
	materialDesc->elementCount = elementCount;
	if (elementCount > 0)
	{
		materialDesc->elements = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsMaterialElement,
			elementCount);
		DS_ASSERT(materialDesc->elements);
		memcpy(materialDesc->elements, elements, sizeof(dsMaterialElement)*elementCount);

		for (uint32_t i = 0; i < elementCount; ++i)
		{
			size_t nameLen = strlen(elements[i].name) + 1;
			char* nameCopy = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
			DS_ASSERT(nameCopy);
			memcpy(nameCopy, elements[i].name, nameLen);
			materialDesc->elements[i].name = nameCopy;
		}
	}

	return materialDesc;
}

bool dsMTLMaterialDesc_destroy(dsResourceManager* resourceManager, dsMaterialDesc* materialDesc)
{
	DS_UNUSED(resourceManager);
	if (materialDesc->allocator)
		DS_VERIFY(dsAllocator_free(materialDesc->allocator, materialDesc));
	return true;
}
