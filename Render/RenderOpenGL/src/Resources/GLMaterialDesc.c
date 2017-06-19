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

#include "Resources/GLMaterialDesc.h"
#include "Resources/GLResource.h"
#include "Resources/GLShaderVariableGroupDesc.h"
#include "Types.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <string.h>

dsMaterialDesc* dsGLMaterialDesc_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	const dsMaterialElement* elements, uint32_t elementCount)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(allocator);
	DS_ASSERT(elements || elementCount == 0);

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsGLMaterialDesc)) +
		DS_ALIGNED_SIZE(elementCount*sizeof(dsMaterialElement));
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsGLMaterialDesc* materialDesc = (dsGLMaterialDesc*)dsAllocator_alloc(
		(dsAllocator*)&bufferAlloc, sizeof(dsGLMaterialDesc));
	DS_ASSERT(materialDesc);

	dsMaterialDesc* baseMaterialDesc = (dsMaterialDesc*)materialDesc;
	baseMaterialDesc->resourceManager = resourceManager;
	baseMaterialDesc->allocator = dsAllocator_keepPointer(allocator);
	baseMaterialDesc->elementCount = elementCount;
	if (elementCount > 0)
	{
		baseMaterialDesc->elements = (dsMaterialElement*)dsAllocator_alloc(
			(dsAllocator*)&bufferAlloc, elementCount*sizeof(dsMaterialElement));
		DS_ASSERT(baseMaterialDesc->elements);
		memcpy(baseMaterialDesc->elements, elements, elementCount*sizeof(dsMaterialElement));
	}
	else
		baseMaterialDesc->elements = NULL;

	dsGLResource_initialize(&materialDesc->resource);
	return baseMaterialDesc;
}

static bool destroyImpl(dsMaterialDesc* materialDesc)
{
	if (materialDesc->allocator)
		return dsAllocator_free(materialDesc->allocator, materialDesc);

	return true;
}

bool dsGLMaterialDesc_destroy(dsResourceManager* resourceManager, dsMaterialDesc* materialDesc)
{
	DS_UNUSED(resourceManager);
	DS_ASSERT(materialDesc);

	dsGLMaterialDesc* glMaterialDesc = (dsGLMaterialDesc*)materialDesc;
	if (dsGLResource_destroy(&glMaterialDesc->resource))
		return destroyImpl(materialDesc);

	return true;
}

void dsGLMaterialDesc_addInternalRef(dsMaterialDesc* materialDesc)
{
	DS_ASSERT(materialDesc);
	dsGLMaterialDesc* glMaterialDesc = (dsGLMaterialDesc*)materialDesc;
	dsGLResource_addRef(&glMaterialDesc->resource);

	for (uint32_t i = 0; i < materialDesc->elementCount; ++i)
	{
		dsShaderVariableGroupDesc* groupDesc =
			(dsShaderVariableGroupDesc*)materialDesc->elements[i].shaderVariableGroupDesc;
		if (groupDesc)
			dsGLShaderVariableGroupDesc_addInternalRef(groupDesc);
	}
}

void dsGLMaterialDesc_freeInternalRef(dsMaterialDesc* materialDesc)
{
	DS_ASSERT(materialDesc);
	for (uint32_t i = 0; i < materialDesc->elementCount; ++i)
	{
		dsShaderVariableGroupDesc* groupDesc =
			(dsShaderVariableGroupDesc*)materialDesc->elements[i].shaderVariableGroupDesc;
		if (groupDesc)
			dsGLShaderVariableGroupDesc_freeInternalRef(groupDesc);
	}

	dsGLMaterialDesc* glMaterialDesc = (dsGLMaterialDesc*)materialDesc;
	if (dsGLResource_freeRef(&glMaterialDesc->resource))
		destroyImpl(materialDesc);
}
