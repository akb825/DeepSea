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

#include "Resources/GLFramebuffer.h"

#include "AnyGL/AnyGL.h"
#include "AnyGL/gl.h"
#include "Resources/GLResource.h"
#include "GLHelpers.h"
#include "GLRendererInternal.h"
#include "Types.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <string.h>

dsFramebuffer* dsGLFramebuffer_create(dsResourceManager* resourceManager,
	dsAllocator* allocator, const dsFramebufferSurface* surfaces, uint32_t surfaceCount,
	uint32_t width, uint32_t height, uint32_t layers)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(allocator);
	DS_ASSERT(surfaces || surfaceCount == 0);

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsGLFramebuffer)) +
		DS_ALIGNED_SIZE(sizeof(dsFramebufferSurface)*surfaceCount);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAllocator;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAllocator, buffer, fullSize));
	dsGLFramebuffer* framebuffer = (dsGLFramebuffer*)dsAllocator_alloc(
		(dsAllocator*)&bufferAllocator, sizeof(dsGLFramebuffer));
	DS_ASSERT(framebuffer);

	dsFramebuffer* baseFramebuffer = (dsFramebuffer*)framebuffer;
	baseFramebuffer->resourceManager = resourceManager;
	baseFramebuffer->allocator = dsAllocator_keepPointer(allocator);
	if (surfaceCount > 0)
	{
		baseFramebuffer->surfaces = (dsFramebufferSurface*)dsAllocator_alloc(
			(dsAllocator*)&bufferAllocator, sizeof(dsFramebufferSurface)*surfaceCount);
		DS_ASSERT(baseFramebuffer->surfaces);
		memcpy(baseFramebuffer->surfaces, surfaces, sizeof(dsFramebufferSurface)*surfaceCount);
	}
	else
		baseFramebuffer->surfaces = NULL;
	baseFramebuffer->surfaceCount = surfaceCount;
	baseFramebuffer->width = width;
	baseFramebuffer->height = height;
	baseFramebuffer->layers = layers;

	dsGLResource_initialize(&framebuffer->resource);
	framebuffer->framebufferId = 0;
	framebuffer->defaultFramebuffer = true;
	for (uint32_t i = 0; i < surfaceCount; ++i)
	{
		switch (surfaces[i].surfaceType)
		{
			case dsFramebufferSurfaceType_Offscreen:
			case dsFramebufferSurfaceType_Renderbuffer:
				framebuffer->defaultFramebuffer = false;
				break;
			default:
				break;
		}
	}

	return baseFramebuffer;
}

static bool destroyImpl(dsFramebuffer* framebuffer)
{
	dsGLFramebuffer* glFramebuffer = (dsGLFramebuffer*)framebuffer;
	dsGLRenderer_destroyFbo(framebuffer->resourceManager->renderer, glFramebuffer->framebufferId,
		glFramebuffer->fboContext);

	if (framebuffer->allocator)
		return dsAllocator_free(framebuffer->allocator, framebuffer);

	return true;
}

bool dsGLFramebuffer_destroy(dsResourceManager* resourceManager, dsFramebuffer* framebuffer)
{
	DS_UNUSED(resourceManager);
	DS_ASSERT(framebuffer);

	dsGLFramebuffer* glBuffer = (dsGLFramebuffer*)framebuffer;
	if (dsGLResource_destroy(&glBuffer->resource))
		return destroyImpl(framebuffer);

	return true;
}

void dsGLFramebuffer_bind(dsFramebuffer* framebuffer)
{
	dsGLFramebuffer* glFramebuffer = (dsGLFramebuffer*)framebuffer;
	if (glFramebuffer->defaultFramebuffer)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return;
	}

	// Framebuffer objects are tied to specific contexts.
	dsGLRenderer* renderer = (dsGLRenderer*)framebuffer->resourceManager->renderer;
	if (!glFramebuffer->framebufferId || glFramebuffer->fboContext != renderer->contextCount)
		glGenVertexArrays(1, &glFramebuffer->framebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, glFramebuffer->framebufferId);
}

void dsGLFramebuffer_addInternalRef(dsFramebuffer* framebuffer)
{
	DS_ASSERT(framebuffer);
	dsGLFramebuffer* glBuffer = (dsGLFramebuffer*)framebuffer;
	dsGLResource_addRef(&glBuffer->resource);
}

void dsGLFramebuffer_freeInternalRef(dsFramebuffer* framebuffer)
{
	DS_ASSERT(framebuffer);
	dsGLFramebuffer* glBuffer = (dsGLFramebuffer*)framebuffer;
	if (dsGLResource_freeRef(&glBuffer->resource))
		destroyImpl(framebuffer);
}
