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

#include "VkProcessResourceList.h"
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/Lifetime.h>
#include <DeepSea/Core/Assert.h>
#include <string.h>

void dsVkProcessResourceList_initialize(dsVkProcessResourceList* resources, dsAllocator* allocator)
{
	memset(resources, 0, sizeof(*resources));
	DS_ASSERT(allocator->freeFunc);
	resources->allocator = allocator;
}

bool dsVkProcessResourceList_addBuffer(dsVkProcessResourceList* resources,
	dsVkGfxBufferData* buffer)
{
	uint32_t index = resources->bufferCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(resources->allocator, resources->buffers, resources->bufferCount,
		resources->maxBuffers, 1))
	{
		return false;
	}

	resources->buffers[index] = dsLifetime_addRef(buffer->lifetime);
	return true;
}

bool dsVkProcessResourceList_addTexture(dsVkProcessResourceList* resources, dsTexture* texture)
{
	uint32_t index = resources->textureCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(resources->allocator, resources->textures, resources->textureCount,
		resources->maxTextures, 1))
	{
		return false;
	}

	dsVkTexture* vkTexture = (dsVkTexture*)texture;
	resources->textures[index] = dsLifetime_addRef(vkTexture->lifetime);
	return true;
}

bool dsVkProcessResourceList_addRenderbuffer(dsVkProcessResourceList* resources,
	dsRenderbuffer* renderbuffer)
{
	uint32_t index = resources->renderbufferCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(resources->allocator, resources->renderbuffers,
		resources->renderbufferCount, resources->maxRenderbuffers, 1))
	{
		return false;
	}

	resources->renderbuffers[index] = renderbuffer;
	return true;
}

bool dsVkProcessResourceList_addRenderSurface(dsVkProcessResourceList* resources,
	dsVkRenderSurfaceData* surface)
{
	uint32_t index = resources->renderSurfaceCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(resources->allocator, resources->renderSurfaces,
		resources->renderSurfaceCount, resources->maxRenderSurfaces, 1))
	{
		return false;
	}

	resources->renderSurfaces[index] = surface;
	return true;
}

void dsVkProcessResourceList_clear(dsVkProcessResourceList* resources)
{
	for (uint32_t i = 0; i < resources->bufferCount; ++i)
		dsLifetime_freeRef(resources->buffers[i]);
	resources->bufferCount = 0;

	for (uint32_t i = 0; i < resources->textureCount; ++i)
		dsLifetime_freeRef(resources->textures[i]);
	resources->textureCount = 0;

	resources->renderbufferCount = 0;
	resources->renderSurfaceCount = 0;
}

void dsVkProcessResourceList_shutdown(dsVkProcessResourceList* resources)
{
	dsVkProcessResourceList_clear(resources);
	DS_VERIFY(dsAllocator_free(resources->allocator, resources->buffers));
	DS_VERIFY(dsAllocator_free(resources->allocator, resources->textures));
	DS_VERIFY(dsAllocator_free(resources->allocator, resources->renderbuffers));
	DS_VERIFY(dsAllocator_free(resources->allocator, resources->renderSurfaces));
}
