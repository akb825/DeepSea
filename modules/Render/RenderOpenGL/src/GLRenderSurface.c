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

#include "GLRenderSurface.h"

#include "Platform/GLPlatform.h"
#include "GLCommandBuffer.h"
#include "GLRendererInternal.h"
#include "GLTypes.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <string.h>

dsRenderSurface* dsGLRenderSurface_create(dsRenderer* renderer, dsAllocator* allocator,
	const char* name, void* osHandle, dsRenderSurfaceType type, dsRenderSurfaceUsage usage)
{
	DS_ASSERT(renderer);
	DS_ASSERT(allocator);
	DS_ASSERT(name);

	dsGLRenderer* glRenderer = (dsGLRenderer*)renderer;
	void* display = glRenderer->options.display;
	void* glSurface = dsCreateGLSurface(allocator, display, glRenderer->renderConfig, type,
		osHandle);
	if (!glSurface)
	{
		errno = EPERM;
		return NULL;
	}

	size_t nameLen = strlen(name) + 1;
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsGLRenderSurface)) + DS_ALIGNED_SIZE(nameLen);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
	{
		dsDestroyGLSurface(display, type, glSurface);
		return NULL;
	}

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsGLRenderSurface* renderSurface = DS_ALLOCATE_OBJECT(&bufferAlloc, dsGLRenderSurface);
	DS_ASSERT(renderSurface);

	dsRenderSurface* baseSurface = (dsRenderSurface*)renderSurface;
	baseSurface->renderer = renderer;
	baseSurface->allocator = dsAllocator_keepPointer(allocator);
	baseSurface->name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
	DS_ASSERT(baseSurface->name);
	memcpy((void*)baseSurface->name, name, nameLen);
	baseSurface->surfaceType = type;
	baseSurface->usage = usage;
	baseSurface->rotation = dsRenderSurfaceRotation_0;
	DS_VERIFY(dsGetGLSurfaceSize(&baseSurface->width, &baseSurface->height, display, type,
		glSurface));
	baseSurface->preRotateWidth = baseSurface->width;
	baseSurface->preRotateHeight = baseSurface->height;

	renderSurface->glSurface = glSurface;
	return baseSurface;
}

bool dsGLRenderSurface_update(dsRenderer* renderer, dsRenderSurface* renderSurface)
{
	DS_ASSERT(renderer);
	DS_ASSERT(renderSurface);

	dsGLRenderer* glRenderer = (dsGLRenderer*)renderer;
	void* display = glRenderer->options.display;

	uint32_t width, height;
	DS_VERIFY(dsGetGLSurfaceSize(&width, &height, display, renderSurface->surfaceType,
		((dsGLRenderSurface*)renderSurface)->glSurface));
	if (width == renderSurface->width && height == renderSurface->height)
		return false;

	renderSurface->width = width;
	renderSurface->height = height;
	renderSurface->preRotateWidth = width;
	renderSurface->preRotateHeight = height;
	return true;
}

bool dsGLRenderSurface_beginDraw(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderSurface* renderSurface)
{
	DS_UNUSED(renderer);
	DS_ASSERT(commandBuffer);
	DS_ASSERT(renderSurface);

	const dsGLRenderSurface* glSurface = (const dsGLRenderSurface*)renderSurface;
	if (!dsGLCommandBuffer_beginRenderSurface(commandBuffer, glSurface->glSurface))
	{
		return false;
	}
	return true;
}

bool dsGLRenderSurface_endDraw(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderSurface* renderSurface)
{
	DS_UNUSED(renderer);
	DS_ASSERT(commandBuffer);
	DS_ASSERT(renderSurface);

	return dsGLCommandBuffer_endRenderSurface(commandBuffer,
		((const dsGLRenderSurface*)renderSurface)->glSurface);
}

bool dsGLRenderSurface_swapBuffers(dsRenderer* renderer, dsRenderSurface** renderSurfaces,
	uint32_t count)
{
	DS_ASSERT(renderer);
	DS_ASSERT(renderSurfaces);
	DS_ASSERT(count > 0);

	// Since swap buffers may block, guarantee that the current commands are flushed first.
	glFlush();
	dsSwapGLBuffers(((dsGLRenderer*)renderer)->options.display, renderSurfaces, count,
		renderer->vsync);
	return true;
}

bool dsGLRenderSurface_destroy(dsRenderer* renderer, dsRenderSurface* renderSurface)
{
	DS_ASSERT(renderer);
	DS_ASSERT(renderSurface);

	void* display = ((dsGLRenderer*)renderer)->options.display;
	void* glSurface = ((dsGLRenderSurface*)renderSurface)->glSurface;
	DS_ASSERT(glSurface);
	dsGLRenderer_destroySurface(renderer, glSurface);
	dsDestroyGLSurface(display, renderSurface->surfaceType, glSurface);
	if (renderSurface->allocator)
		return dsAllocator_free(renderSurface->allocator, renderSurface);
	return true;
}
