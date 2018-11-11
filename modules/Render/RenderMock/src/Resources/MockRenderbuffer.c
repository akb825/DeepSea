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

#include "Resources/MockRenderbuffer.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Render/Resources/GfxFormat.h>

dsRenderbuffer* dsMockRenderbuffer_create(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsRenderbufferUsage usage, dsGfxFormat format, uint32_t width,
	uint32_t height, uint32_t samples)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(allocator);
	DS_ASSERT(dsGfxFormat_offscreenSupported(resourceManager, format));

	dsRenderbuffer* renderbuffer = DS_ALLOCATE_OBJECT(allocator, dsRenderbuffer);
	if (!renderbuffer)
		return NULL;

	renderbuffer->resourceManager = resourceManager;
	renderbuffer->allocator = dsAllocator_keepPointer(allocator);
	renderbuffer->usage = usage;
	renderbuffer->format = format;
	renderbuffer->width = width;
	renderbuffer->height = height;
	renderbuffer->samples = samples;
	return renderbuffer;
}

bool dsMockRenderbuffer_destroy(dsResourceManager* resourceManager, dsRenderbuffer* renderbuffer)
{
	DS_UNUSED(resourceManager);
	DS_ASSERT(renderbuffer);
	if (renderbuffer->allocator)
		return dsAllocator_free(renderbuffer->allocator, renderbuffer);
	return true;
}
