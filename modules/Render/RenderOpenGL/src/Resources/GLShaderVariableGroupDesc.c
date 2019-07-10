/*
 * Copyright 2017-2019 Aaron Barany
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

#include "Resources/GLShaderVariableGroupDesc.h"

#include "Resources/GLResource.h"
#include "GLTypes.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Render/Resources/MaterialType.h>
#include <string.h>

dsShaderVariableGroupDesc* dsGLShaderVariableGroupDesc_create(dsResourceManager* resourceManager,
	dsAllocator* allocator, const dsShaderVariableElement* elements, uint32_t elementCount)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(allocator);
	DS_ASSERT(elements);

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsGLShaderVariableGroupDesc)) +
		DS_ALIGNED_SIZE(elementCount*sizeof(dsShaderVariableElement)) +
		DS_ALIGNED_SIZE(elementCount*sizeof(dsShaderVariablePos));
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsGLShaderVariableGroupDesc* groupDesc = DS_ALLOCATE_OBJECT(&bufferAlloc,
		dsGLShaderVariableGroupDesc);
	DS_ASSERT(groupDesc);

	dsShaderVariableGroupDesc* baseGroupDesc = (dsShaderVariableGroupDesc*)groupDesc;
	baseGroupDesc->resourceManager = resourceManager;
	baseGroupDesc->allocator = dsAllocator_keepPointer(allocator);
	baseGroupDesc->elementCount = elementCount;
	baseGroupDesc->elements = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsShaderVariableElement,
		elementCount);
	DS_ASSERT(baseGroupDesc->elements);
	memcpy(baseGroupDesc->elements, elements,
		elementCount*sizeof(dsShaderVariableElement));

	baseGroupDesc->positions = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsShaderVariablePos,
		elementCount);
	DS_ASSERT(baseGroupDesc->positions);
	size_t curSize = 0;
	for (uint32_t i = 0; i < elementCount; ++i)
	{
		baseGroupDesc->positions[i].offset = (uint32_t)dsMaterialType_addElementBlockSize(
			&curSize, elements[i].type, elements[i].count);
		if (elements[i].count > 0)
		{
			baseGroupDesc->positions[i].stride = dsMaterialType_blockSize(elements[i].type,
				true);
		}
		else
			baseGroupDesc->positions[i].stride = 0;
		baseGroupDesc->positions[i].matrixColStride = dsMaterialType_blockAlignment(
			dsMaterialType_matrixColumnType(elements[i].type), true);
	}

	dsGLResource_initialize(&groupDesc->resource);
	return baseGroupDesc;
}

static bool destroyImpl(dsShaderVariableGroupDesc* groupDesc)
{
	if (groupDesc->allocator)
		return dsAllocator_free(groupDesc->allocator, groupDesc);

	return true;
}

bool dsGLShaderVariableGroupDesc_destroy(dsResourceManager* resourceManager,
	dsShaderVariableGroupDesc* groupDesc)
{
	DS_UNUSED(resourceManager);
	DS_ASSERT(groupDesc);

	dsGLShaderVariableGroupDesc* glGroupDesc = (dsGLShaderVariableGroupDesc*)groupDesc;
	if (dsGLResource_destroy(&glGroupDesc->resource))
		return destroyImpl(groupDesc);

	return true;
}

void dsGLShaderVariableGroupDesc_addInternalRef(dsShaderVariableGroupDesc* groupDesc)
{
	DS_ASSERT(groupDesc);
	dsGLShaderVariableGroupDesc* glGroupDesc = (dsGLShaderVariableGroupDesc*)groupDesc;
	dsGLResource_addRef(&glGroupDesc->resource);
}

void dsGLShaderVariableGroupDesc_freeInternalRef(dsShaderVariableGroupDesc* groupDesc)
{
	DS_ASSERT(groupDesc);
	dsGLShaderVariableGroupDesc* glGroupDesc = (dsGLShaderVariableGroupDesc*)groupDesc;
	if (dsGLResource_freeRef(&glGroupDesc->resource))
		destroyImpl(groupDesc);
}
