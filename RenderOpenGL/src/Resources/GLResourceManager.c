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

#include "Resources/GLResourceManager.h"

#include "AnyGL/AnyGL.h"
#include "AnyGL/gl.h"
#include "Platform/Platform.h"
#include "Resources/GLGfxBuffer.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Thread/Mutex.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/ResourceManager.h>
#include <stdint.h>
#include <string.h>

static size_t dsGLResourceManager_fullAllocSize(const dsOpenGLOptions* options)
{
	return DS_ALIGNED_SIZE(sizeof(dsGLResourceManager)) +
		DS_ALIGNED_SIZE(options->maxResourceThreads*sizeof(dsResourceContext)) +
		dsMutex_fullAllocSize();
}

static dsGfxBufferUsage getSupportedBuffers(void)
{
	dsGfxBufferUsage supportedBuffers = (dsGfxBufferUsage)(dsGfxBufferUsage_Vertex |
		dsGfxBufferUsage_Index | dsGfxBufferUsage_CopyTo | dsGfxBufferUsage_CopyFrom);
	if (AnyGL_atLeastVersion(4, 0, false) || AnyGL_atLeastVersion(3, 1, true) ||
		AnyGL_ARB_draw_indirect)
	{
		supportedBuffers = (dsGfxBufferUsage)(supportedBuffers | dsGfxBufferUsage_IndirectDraw);
	}

	if (AnyGL_atLeastVersion(4, 3, false) || AnyGL_atLeastVersion(3, 1, true) ||
		AnyGL_ARB_compute_shader)
	{
		supportedBuffers = (dsGfxBufferUsage)(supportedBuffers | dsGfxBufferUsage_IndirectDispatch);
	}

	if (AnyGL_atLeastVersion(3, 1, false) || AnyGL_atLeastVersion(3, 0, true) ||
		AnyGL_ARB_texture_buffer_object || AnyGL_EXT_texture_buffer_object)
	{
		supportedBuffers = (dsGfxBufferUsage)(supportedBuffers | dsGfxBufferUsage_Image);
	}

	if (AnyGL_atLeastVersion(3, 1, false) || AnyGL_atLeastVersion(3, 0, true) ||
		AnyGL_ARB_uniform_buffer_object)
	{
		supportedBuffers = (dsGfxBufferUsage)(supportedBuffers | dsGfxBufferUsage_UniformBlock);
	}

	if (AnyGL_atLeastVersion(4, 3, false) || AnyGL_atLeastVersion(3, 1, true) ||
		AnyGL_ARB_shader_storage_buffer_object)
	{
		supportedBuffers = (dsGfxBufferUsage)(supportedBuffers | dsGfxBufferUsage_UniformBuffer |
			dsGfxBufferUsage_MutableImage);
	}

	return supportedBuffers;
}

static dsGfxBufferMapSupport getBufferMapSupport(void)
{
	if (!ANYGL_SUPPORTED(glMapBuffer))
		return dsGfxBufferMapSupport_None;
	else if (!ANYGL_SUPPORTED(glMapBufferRange))
		return dsGfxBufferMapSupport_Full;
	else if (AnyGL_atLeastVersion(4, 4, true) || AnyGL_ARB_buffer_storage)
		return dsGfxBufferMapSupport_Persistent;

	return dsGfxBufferMapSupport_Range;
}

static dsResourceContext* createResourceContext(dsResourceManager* resourceManager)
{
	DS_ASSERT(resourceManager);

	dsGLResourceManager* glResourceManager = (dsGLResourceManager*)resourceManager;
	DS_VERIFY(dsMutex_lock(glResourceManager->mutex));
	dsResourceContext* context = NULL;
	for (uint32_t i = 0; i < resourceManager->maxResourceContexts; ++i)
	{
		if (!glResourceManager->resourceContexts[i].claimed)
		{
			context = glResourceManager->resourceContexts + i;
			context->claimed = true;
			break;
		}
	}
	DS_VERIFY(dsMutex_unlock(glResourceManager->mutex));

	// This should only be null in case of a bug or somebody manually messing with the members.
	DS_ASSERT(context);
	const dsOpenGLOptions* options = &((dsGLRenderer*)resourceManager->renderer)->options;
	DS_VERIFY(dsBindGLContext(options->display, context->context, context->dummySurface));
	return context;
}

static bool destroyResourceContext(dsResourceManager* resourceManager, dsResourceContext* context)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(context);

	const dsOpenGLOptions* options = &((dsGLRenderer*)resourceManager->renderer)->options;
	DS_VERIFY(dsBindGLContext(options->display, NULL, NULL));

	dsGLResourceManager* glResourceManager = (dsGLResourceManager*)resourceManager;
	DS_VERIFY(dsMutex_lock(glResourceManager->mutex));
	context->claimed = false;
	DS_VERIFY(dsMutex_unlock(glResourceManager->mutex));

	return true;
}

dsGLResourceManager* dsGLResourceManager_create(dsAllocator* allocator, dsGLRenderer* renderer)
{
	DS_ASSERT(allocator);
	DS_ASSERT(renderer);

	const dsOpenGLOptions* options = &renderer->options;
	size_t bufferSize = dsGLResourceManager_fullAllocSize(options);
	void* buffer = dsAllocator_alloc(allocator, bufferSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, bufferSize));

	dsGLResourceManager* resourceManager = (dsGLResourceManager*)dsAllocator_alloc(
		(dsAllocator*)&bufferAlloc, sizeof(dsGLResourceManager));
	DS_ASSERT(resourceManager);
	dsResourceManager* baseResourceManager = (dsResourceManager*)resourceManager;
	DS_VERIFY(dsResourceManager_initialize(baseResourceManager));

	if (options->maxResourceThreads > 0)
	{
		resourceManager->resourceContexts = (dsResourceContext*)dsAllocator_alloc(
			(dsAllocator*)&bufferAlloc, sizeof(dsResourceContext)*options->maxResourceThreads);
		DS_ASSERT(resourceManager->resourceContexts);
		memset(resourceManager->resourceContexts, 0,
			sizeof(dsResourceContext)*options->maxResourceThreads);
	}
	else
		resourceManager->resourceContexts = NULL;

	resourceManager->mutex = dsMutex_create((dsAllocator*)&bufferAlloc, "Resource Manager");
	DS_ASSERT(resourceManager->mutex);

	baseResourceManager->renderer = (dsRenderer*)renderer;
	baseResourceManager->allocator = dsAllocator_keepPointer(allocator);
	baseResourceManager->maxResourceContexts = options->maxResourceThreads;

	for (uint8_t i = 0; i < options->maxResourceThreads; ++i)
	{
		dsResourceContext* resourceContext = resourceManager->resourceContexts + i;
		resourceContext->context = dsCreateGLContext(allocator, options->display,
			renderer->sharedConfig, renderer->sharedContext);
		if (!resourceContext->context)
		{
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Couldn't create GL context.");
			dsGLResourceManager_destroy(resourceManager);
			return NULL;
		}

		resourceContext->dummySurface = dsCreateDummyGLSurface(allocator, options->display,
			renderer->sharedConfig, &resourceManager->resourceContexts[i].dummyOsSurface);
		if (!resourceContext->dummySurface)
		{
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Couldn't create dummy GL surface.");
			dsGLResourceManager_destroy(resourceManager);
			return NULL;
		}
	}

	// Resource contexts
	baseResourceManager->createResourceContextFunc = &createResourceContext;
	baseResourceManager->destroyResourceContextFunc = &destroyResourceContext;

	// Buffers
	if (AnyGL_atLeastVersion(4, 2, false) || AnyGL_ARB_map_buffer_alignment)
	{
		glGetIntegerv(GL_MIN_MAP_BUFFER_ALIGNMENT,
			(GLint*)&baseResourceManager->minMappingAlignment);
	}
	baseResourceManager->supportedBuffers = getSupportedBuffers();
	baseResourceManager->bufferMapSupport = getBufferMapSupport();
	baseResourceManager->canCopyBuffers = ANYGL_SUPPORTED(glCopyBufferSubData);

	if (AnyGL_atLeastVersion(1, 0, false) || AnyGL_atLeastVersion(3, 0, true) ||
		AnyGL_OES_element_index_uint)
	{
		baseResourceManager->maxIndexBits = 32;
	}
	else
		baseResourceManager->maxIndexBits = 16;

	if (baseResourceManager->supportedBuffers & dsGfxBufferUsage_UniformBlock)
	{
		if (ANYGL_SUPPORTED(glGetInteger64v))
		{
			GLint64 maxSize = 0;
			glGetInteger64v(GL_MAX_UNIFORM_BLOCK_SIZE, &maxSize);
			baseResourceManager->maxUniformBlockSize = (size_t)(dsMin(maxSize,
				SIZE_MAX));
		}
		else
		{
			GLint maxSize = 0;
			glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxSize);
			baseResourceManager->maxUniformBlockSize = maxSize;
		}
	}

	baseResourceManager->createBufferFunc = &dsGLGfxBuffer_create;
	baseResourceManager->destroyBufferFunc = &dsGLGfxBuffer_destroy;
	if (baseResourceManager->bufferMapSupport != dsGfxBufferMapSupport_None)
	{
		baseResourceManager->mapBufferFunc = &dsGLGfxBuffer_map;
		baseResourceManager->unmapBufferFunc = &dsGLGfxBuffer_unmap;
		if (baseResourceManager->bufferMapSupport == dsGfxBufferMapSupport_Persistent)
		{
			baseResourceManager->flushBufferFunc = &dsGLGfxBuffer_flush;
			baseResourceManager->invalidateBufferFunc = &dsGLGfxBuffer_invalidate;
		}
	}

	return resourceManager;
}

void dsGLResourceManager_destroy(dsGLResourceManager* resourceManager)
{
	if (!resourceManager)
		return;

	dsResourceManager* baseResourceManager = (dsResourceManager*)resourceManager;
	const dsOpenGLOptions* options = &((dsGLRenderer*)baseResourceManager->renderer)->options;
	for (uint8_t i = 0; i < ((dsResourceManager*)resourceManager)->maxResourceContexts; ++i)
	{
		dsResourceContext* resourceContext = resourceManager->resourceContexts + i;
		dsDestroyGLContext(options->display, resourceContext->context);
		dsDestroyDummyGLSurface(options->display, resourceContext->dummySurface,
			resourceContext->dummyOsSurface);
	}

	dsMutex_destroy(resourceManager->mutex);
	dsResourceManager_shutdown((dsResourceManager*)resourceManager);
	if (((dsResourceManager*)resourceManager)->allocator)
		dsAllocator_free(((dsResourceManager*)resourceManager)->allocator, resourceManager);
}
