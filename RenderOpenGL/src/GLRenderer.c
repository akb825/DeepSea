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

#include <DeepSea/RenderOpenGL/GLRenderer.h>
#include "GLRendererInternal.h"

#include "AnyGL/AnyGL.h"
#include "AnyGL/gl.h"
#include "Platform/Platform.h"
#include "Resources/GLResourceManager.h"
#include "GLMainCommandBuffer.h"
#include "Types.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Thread/Mutex.h>
#include <DeepSea/Core/Thread/Thread.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Renderer.h>
#include <string.h>

static dsGfxFormat getColorFormat(const dsOpenGLOptions* options)
{
	if (options->redBits == 8 && options->greenBits == 0 && options->blueBits == 0)
	{
		if (options->alphaBits == 8)
		{
			if (options->srgb)
				return dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_SRGB);
			else
				return dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
		}
		else
		{
			if (options->srgb)
				return dsGfxFormat_decorate(dsGfxFormat_R8G8B8, dsGfxFormat_SRGB);
			else
				return dsGfxFormat_decorate(dsGfxFormat_R8G8B8, dsGfxFormat_UNorm);
		}
	}
	else if (options->redBits == 5 && options->greenBits == 6 && options->blueBits == 5 &&
		options->alphaBits == 0 && !options->srgb)
	{
		return dsGfxFormat_decorate(dsGfxFormat_R5G6B5, dsGfxFormat_UNorm);
	}

	return dsGfxFormat_Unknown;
}

static dsGfxFormat getDepthFormat(const dsOpenGLOptions* options)
{
	if (options->depthBits == 24)
		return dsGfxFormat_D24S8;
	else if (options->depthBits == 16 && options->stencilBits == 0)
		return dsGfxFormat_D16;

	return dsGfxFormat_Unknown;
}

static size_t dsGLRenderer_fullAllocSize(void)
{
	return DS_ALIGNED_SIZE(sizeof(dsGLRenderer)) + dsMutex_fullAllocSize();
}

static bool hasRequiredFunctions(void)
{
	if (!ANYGL_SUPPORTED(glGenBuffers) || !ANYGL_SUPPORTED(glGenFramebuffers))
		return false;

	return true;
}

void dsGLRenderer_defaultOptions(dsOpenGLOptions* options)
{
	if (!options)
		return;

	options->display = NULL;
	options->redBits = 8;
	options->greenBits = 8;
	options->blueBits = 8;
	options->alphaBits = 0;
	options->depthBits = 24;
	options->stencilBits = 8;
	options->samples = 4;
	options->doubleBuffer = true;
	options->srgb = false;
	options->stereoscopic = false;
	options->accelerated = -1;
	options->debug = ANYGL_ALLOW_DEBUG;
	options->maxResourceThreads = 0;
}

dsRenderer* dsGLRenderer_create(dsAllocator* allocator, const dsOpenGLOptions* options)
{
	if (!allocator || !options)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!AnyGL_initialize())
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Cannot initialize GL.");
		return NULL;
	}

	dsGfxFormat colorFormat = getColorFormat(options);
	if (!dsGfxFormat_isValid(colorFormat))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Invalid color format.");
		AnyGL_shutdown();
		return NULL;
	}

	dsGfxFormat depthFormat = getDepthFormat(options);

	size_t bufferSize = dsGLRenderer_fullAllocSize();
	void* buffer = dsAllocator_alloc(allocator, bufferSize);
	if (!buffer)
	{
		AnyGL_shutdown();
		return NULL;
	}

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, bufferSize));
	dsGLRenderer* renderer = (dsGLRenderer*)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
		sizeof(dsGLRenderer));
	DS_ASSERT(renderer);
	memset(renderer, 0, sizeof(*renderer));
	dsRenderer* baseRenderer = (dsRenderer*)renderer;

	DS_VERIFY(dsRenderer_initialize(baseRenderer));
	baseRenderer->allocator = dsAllocator_keepPointer(allocator);

	renderer->options = *options;
	if (!renderer->options.display)
	{
		renderer->options.display = dsGetGLDisplay();
		renderer->releaseDisplay = true;
	}
	else
		renderer->releaseDisplay = false;

	void* display = renderer->options.display;
	renderer->sharedConfig = dsCreateGLConfig(allocator, display, options, false);
	renderer->renderConfig = dsCreateGLConfig(allocator, display, options, true);
	if (!renderer->sharedConfig || !renderer->renderConfig)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Couldn't create GL configuration.");
		dsGLRenderer_destroy(baseRenderer);
		return NULL;
	}

	renderer->dummySurface = dsCreateDummyGLSurface(allocator, display, renderer->sharedConfig,
		&renderer->dummyOsSurface);
	if (!renderer->dummySurface)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Couldn't create dummy GL surface.");
		dsGLRenderer_destroy(baseRenderer);
		return NULL;
	}

	renderer->sharedContext = dsCreateGLContext(allocator, display, renderer->sharedConfig,
		NULL);
	if (!renderer->sharedContext)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Couldn't create GL context.");
		dsGLRenderer_destroy(baseRenderer);
		return NULL;
	}

	if (!dsBindGLContext(display, renderer->sharedContext, renderer->dummySurface))
	{
		errno = EPERM;
		dsGLRenderer_destroy(baseRenderer);
		return NULL;
	}

	if (!AnyGL_load())
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Couldn't load GL functions.");
		dsGLRenderer_destroy(baseRenderer);
		return NULL;
	}

	if (!hasRequiredFunctions())
	{
		errno = EPERM;
		int major, minor;
		AnyGL_getGLVersion(&major, &minor, NULL);
		DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG, "OpenGL %d.%d is too old.", major, minor);
		dsGLRenderer_destroy(baseRenderer);
		return NULL;
	}

	GLint maxSamples = 0;
	glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
	maxSamples = dsMax(1, maxSamples);
	baseRenderer->maxSurfaceSamples = (uint16_t)maxSamples;
	renderer->options.samples = dsMin(renderer->options.samples, (uint8_t)maxSamples);

	renderer->renderContext = dsCreateGLContext(allocator, display, renderer->renderConfig,
		renderer->sharedContext);
	if (!renderer->renderContext)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Couldn't create GL context.");
		dsGLRenderer_destroy(baseRenderer);
		return NULL;
	}

	renderer->contextMutex = dsMutex_create(allocator, "GL context");
	DS_ASSERT(renderer->contextMutex);

	baseRenderer->resourceManager = (dsResourceManager*)dsGLResourceManager_create(allocator,
		renderer);
	if (!baseRenderer->resourceManager)
	{
		dsGLRenderer_destroy(baseRenderer);
		return NULL;
	}

	baseRenderer->mainCommandBuffer = (dsCommandBuffer*)dsGLMainCommandBuffer_create(baseRenderer,
		allocator);
	if (!baseRenderer->mainCommandBuffer)
	{
		dsGLRenderer_destroy(baseRenderer);
		return NULL;
	}

	baseRenderer->surfaceColorFormat = colorFormat;
	baseRenderer->surfaceDepthStencilFormat = depthFormat;
	baseRenderer->surfaceSamples = options->samples;
	baseRenderer->doubleBuffer = options->doubleBuffer;
	baseRenderer->stereoscopic = options->stereoscopic;

	baseRenderer->supportsInstancedDrawing = ANYGL_SUPPORTED(glVertexAttribDivisor);

	return baseRenderer;
}

void dsGLRenderer_setEnableErrorChecking(dsRenderer* renderer, bool enabled)
{
	if (!renderer)
		return;

	AnyGL_setDebugEnabled(enabled);
}

void dsGLRenderer_destroyVao(dsRenderer* renderer, GLuint vao, uint32_t contextCount)
{
	dsAllocator* allocator = renderer->allocator;
	if (!vao || !allocator)
		return;

	dsGLRenderer* glRenderer = (dsGLRenderer*)renderer;
	if (!dsThread_equal(dsThread_thisThreadId(), renderer->mainThread) &&
		glRenderer->renderContextBound)
	{
		if (contextCount == glRenderer->contextCount)
			glDeleteVertexArrays(1, &vao);
		return;
	}

	dsMutex_lock(glRenderer->contextMutex);
	if (contextCount != glRenderer->contextCount)
	{
		dsMutex_unlock(glRenderer->contextMutex);
		return;
	}

	if (glRenderer->curDestroyVaos >= glRenderer->maxDestroyVaos)
	{
		size_t newMaxVaos = dsMax(16U, glRenderer->maxDestroyVaos*2);
		GLuint* newDestroyVaos = (GLuint*)dsAllocator_alloc(allocator, newMaxVaos*sizeof(GLuint));
		if (!newDestroyVaos)
		{
			dsMutex_unlock(glRenderer->contextMutex);
			return;
		}

		memcpy(newDestroyVaos, glRenderer->destroyVaos, glRenderer->curDestroyVaos*sizeof(GLuint));
		DS_VERIFY(dsAllocator_free(allocator, glRenderer->destroyVaos));
		glRenderer->destroyVaos = newDestroyVaos;
		glRenderer->maxDestroyVaos = newMaxVaos;
	}

	DS_ASSERT(glRenderer->curDestroyVaos < glRenderer->maxDestroyVaos);
	glRenderer->destroyVaos[glRenderer->curDestroyVaos++] = vao;

	dsMutex_unlock(glRenderer->contextMutex);
}

void dsGLRenderer_destroyFbo(dsRenderer* renderer, GLuint fbo, uint32_t contextCount)
{
	dsAllocator* allocator = renderer->allocator;
	if (!fbo || !allocator)
		return;

	dsGLRenderer* glRenderer = (dsGLRenderer*)renderer;
	if (!dsThread_equal(dsThread_thisThreadId(), renderer->mainThread) &&
		glRenderer->renderContextBound)
	{
		if (contextCount == glRenderer->contextCount)
			glDeleteFramebuffers(1, &fbo);
		return;
	}

	dsMutex_lock(glRenderer->contextMutex);
	if (contextCount != glRenderer->contextCount)
	{
		dsMutex_unlock(glRenderer->contextMutex);
		return;
	}

	if (glRenderer->curDestroyVaos >= glRenderer->maxDestroyFbos)
	{
		size_t newMaxFbos = dsMax(16U, glRenderer->maxDestroyFbos*2);
		GLuint* newDestroyFbos = (GLuint*)dsAllocator_alloc(allocator, newMaxFbos*sizeof(GLuint));
		if (!newDestroyFbos)
		{
			dsMutex_unlock(glRenderer->contextMutex);
			return;
		}

		memcpy(newDestroyFbos, glRenderer->destroyFbos, glRenderer->curDestroyFbos*sizeof(GLuint));
		DS_VERIFY(dsAllocator_free(allocator, glRenderer->destroyFbos));
		glRenderer->destroyFbos = newDestroyFbos;
		glRenderer->maxDestroyFbos = newMaxFbos;
	}

	DS_ASSERT(glRenderer->curDestroyFbos < glRenderer->maxDestroyFbos);
	glRenderer->destroyFbos[glRenderer->curDestroyFbos++] = fbo;

	dsMutex_unlock(glRenderer->contextMutex);
}

GLuint dsGLRenderer_tempFramebuffer(dsRenderer* renderer)
{
	dsGLRenderer* glRenderer = (dsGLRenderer*)renderer;
	if (!glRenderer->renderContextBound)
		return 0;

	if (glRenderer->tempFramebuffer)
		return glRenderer->tempFramebuffer;

	glGenFramebuffers(1, &glRenderer->tempFramebuffer);
	return glRenderer->tempFramebuffer;
}

GLuint dsGLRenderer_tempCopyFramebuffer(dsRenderer* renderer)
{
	dsGLRenderer* glRenderer = (dsGLRenderer*)renderer;
	if (!glRenderer->renderContextBound)
		return 0;

	if (glRenderer->tempCopyFramebuffer)
		return glRenderer->tempCopyFramebuffer;

	glGenFramebuffers(1, &glRenderer->tempCopyFramebuffer);
	return glRenderer->tempCopyFramebuffer;
}

void dsGLRenderer_destroy(dsRenderer* renderer)
{
	if (!renderer)
		return;

	dsGLResourceManager_destroy((dsGLResourceManager*)renderer->resourceManager);
	dsGLMainCommandBuffer_destroy((dsGLMainCommandBuffer*)renderer->mainCommandBuffer);

	dsGLRenderer* glRenderer = (dsGLRenderer*)renderer;
	void* display = glRenderer->options.display;
	dsDestroyGLContext(display, glRenderer->renderContext);
	dsDestroyGLContext(display, glRenderer->sharedContext);\
	dsDestroyDummyGLSurface(display, glRenderer->dummySurface, glRenderer->dummyOsSurface);
	dsDestroyGLConfig(display, glRenderer->sharedConfig);
	dsDestroyGLConfig(display, glRenderer->renderConfig);

	if (glRenderer->destroyVaos)
		dsAllocator_free(renderer->allocator, glRenderer->destroyVaos);
	if (glRenderer->destroyFbos)
		dsAllocator_free(renderer->allocator, glRenderer->destroyFbos);
	dsMutex_destroy(glRenderer->contextMutex);

	if (renderer->allocator)
		dsAllocator_free(renderer->allocator, renderer);

	AnyGL_shutdown();
}
