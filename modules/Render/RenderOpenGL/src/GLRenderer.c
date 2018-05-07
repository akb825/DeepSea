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
#include "GLCommandBuffer.h"
#include "GLCommandBufferPool.h"
#include "GLHelpers.h"
#include "GLMainCommandBuffer.h"
#include "GLRenderPass.h"
#include "GLRenderSurface.h"
#include "Types.h"

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/PoolAllocator.h>
#include <DeepSea/Core/Thread/Mutex.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Thread/Thread.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Renderer.h>
#include <string.h>

#define DS_SYNC_POOL_COUNT 100

static void APIENTRY debugOutput(GLenum source, GLenum type, GLuint id, GLenum severity,
	GLsizei length, const GLchar* message, const void* userParam)
{
	DS_UNUSED(type);
	DS_UNUSED(id);
	DS_UNUSED(length);
	DS_UNUSED(userParam);
	const char* tag = DS_RENDER_OPENGL_LOG_TAG;
	switch (source)
	{
		case GL_DEBUG_SOURCE_SHADER_COMPILER:
			tag = "opengl-shader";
			break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
			tag = "opengl-window";
			break;
	}

	dsLogLevel level = dsLogLevel_Info;
	switch (severity)
	{
		case GL_DEBUG_SEVERITY_HIGH:
			level = dsLogLevel_Error;
			break;
		case GL_DEBUG_SEVERITY_MEDIUM:
			level = dsLogLevel_Warning;
			break;
		case GL_DEBUG_SEVERITY_LOW:
			level = dsLogLevel_Info;
			break;
		case GL_DEBUG_SEVERITY_NOTIFICATION:
			level = dsLogLevel_Debug;
			break;
	}

	const char* file;
	const char* function;
	unsigned int line;
	AnyGL_getLastCallsite(&file, &function, &line);
	dsLog_message(level, tag, file, line, function, (const char*)message);
}

static dsGfxFormat getColorFormat(const dsOpenGLOptions* options)
{
	if (options->redBits == 8 && options->greenBits == 8 && options->blueBits == 8)
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
	else if (options->redBits == 10 && options->greenBits == 10 && options->blueBits == 10 &&
		options->alphaBits == 2)
	{
		if (options->srgb)
			return dsGfxFormat_decorate(dsGfxFormat_A2B10G10R10, dsGfxFormat_SRGB);
		else
			return dsGfxFormat_decorate(dsGfxFormat_A2B10G10R10, dsGfxFormat_UNorm);
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

static size_t dsGLRenderer_fullAllocSize(const dsOpenGLOptions* options)
{
	size_t pathLen = options->shaderCacheDir ? strlen(options->shaderCacheDir) + 1 : 0;
	return DS_ALIGNED_SIZE(sizeof(dsGLRenderer)) + dsMutex_fullAllocSize() +
		DS_ALIGNED_SIZE(pathLen);
}

static bool hasRequiredFunctions(void)
{
	if (!ANYGL_SUPPORTED(glGenBuffers) || !ANYGL_SUPPORTED(glGenFramebuffers) ||
		!ANYGL_SUPPORTED(glCreateShader))
	{
		return false;
	}

	return true;
}

static dsPoolAllocator* addPool(dsAllocator* allocator, dsPoolAllocator** pools, uint32_t* curPools,
	uint32_t* maxPools, uint32_t elemSize, uint32_t poolElements)
{
	DS_ASSERT(allocator);
	DS_ASSERT(pools);
	DS_ASSERT(curPools);
	DS_ASSERT(allocator);
	DS_ASSERT(maxPools);
	DS_ASSERT(*pools || *curPools == 0);

	size_t poolSize = dsPoolAllocator_bufferSize(elemSize, poolElements);
	void* poolBuffer = dsAllocator_alloc(allocator, poolSize);
	if (!poolBuffer)
		return NULL;

	size_t index = *curPools;
	if (!DS_RESIZEABLE_ARRAY_ADD(allocator, pools, *curPools, *maxPools, 1))
		return NULL;

	DS_ASSERT(index < *maxPools);
	dsPoolAllocator* pool = *pools + index;
	DS_VERIFY(dsPoolAllocator_initialize(pool, elemSize, poolElements, poolBuffer, poolSize));
	return pool;
}

static void deleteDestroyedObjects(dsGLRenderer* renderer)
{
	if (renderer->curDestroyVaos)
	{
		glDeleteVertexArrays((GLsizei)renderer->curDestroyVaos, renderer->destroyVaos);
		renderer->curDestroyVaos = 0;
	}

	if (renderer->curDestroyFbos)
	{
		glDeleteFramebuffers((GLsizei)renderer->curDestroyFbos, renderer->destroyFbos);
		renderer->curDestroyFbos = 0;
	}
}

static void clearDestroyedObjects(dsGLRenderer* renderer)
{
	renderer->curDestroyVaos = 0;
	renderer->curDestroyFbos = 0;
}

bool dsGLRenderer_beginFrame(dsRenderer* renderer)
{
	dsGLRenderer* glRenderer = (dsGLRenderer*)renderer;
	if (glRenderer->withinFrame)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG,
			"Cannot begin a frame while a frame is currently active.");
		return false;
	}

	if (glRenderer->renderContextBound)
		deleteDestroyedObjects(glRenderer);

	glRenderer->withinFrame = true;
	return true;
}

bool dsGLRenderer_endFrame(dsRenderer* renderer)
{
	dsGLRenderer* glRenderer = (dsGLRenderer*)renderer;
	if (!glRenderer->withinFrame)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG,
			"Cannot end a frame when a frame isn't currently active.");
		return false;
	}

	if (glRenderer->renderContextBound)
		deleteDestroyedObjects(glRenderer);

	glFlush();
	glRenderer->withinFrame = false;
	// Force the context to be re-bound next frame.
	glRenderer->curGLSurface = NULL;
	return true;
}

bool dsGLRenderer_setSurfaceSamples(dsRenderer* renderer, uint32_t samples)
{
	dsGLRenderer* glRenderer = (dsGLRenderer*)renderer;
	if (glRenderer->withinFrame)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG,
			"Cannot set the number of surface samples within a frame.");
		return false;
	}

	samples = dsMin(samples, (uint8_t)renderer->maxSurfaceSamples);
	if (samples == renderer->surfaceSamples)
		return true;

	// Need to re-create the render context.
	DS_ASSERT(glRenderer->renderContext);
	DS_ASSERT(glRenderer->renderConfig);

	void* display = glRenderer->options.display;
	dsOpenGLOptions newOptions = glRenderer->options;
	newOptions.samples = (uint8_t)samples;
	void* newConfig = dsCreateGLConfig(renderer->allocator, display, &newOptions, true);
	if (!newConfig)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Couldn't create OpenGL configuration.");
		return false;
	}

	void* newContext = dsCreateGLContext(renderer->allocator, display, newConfig,
		glRenderer->sharedContext);
	if (!newContext)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Couldn't create OpenGL context.");
		dsDestroyGLConfig(display, newConfig);
		return false;
	}

	DS_VERIFY(dsBindGLContext(display, glRenderer->sharedContext, glRenderer->dummySurface));
	dsDestroyGLContext(display, glRenderer->renderContext);
	dsDestroyGLConfig(display, glRenderer->renderConfig);
	glRenderer->renderConfig = newConfig;
	glRenderer->renderContext = newContext;
	glRenderer->renderContextBound = false;
	glRenderer->renderContextReset = false;
	glRenderer->options.samples = (uint8_t)samples;
	++glRenderer->contextCount;

	// These objects were associated with the now destroyed context.
	clearDestroyedObjects(glRenderer);
	glRenderer->tempFramebuffer = 0;
	glRenderer->tempCopyFramebuffer = 0;
	memset(glRenderer->boundAttributes, 0, sizeof(glRenderer->boundAttributes));

	return true;
}

bool dsGLRenderer_setVsync(dsRenderer* renderer, bool vsync)
{
	renderer->vsync = vsync;
	return true;
}

bool dsGLRenderer_setDefaultAnisotropy(dsRenderer* renderer, float anisotropy)
{
	renderer->defaultAnisotropy = anisotropy;
	return true;
}

bool dsGLRenderer_waitUntilIdle(dsRenderer* renderer)
{
	DS_UNUSED(renderer);
	glFinish();
	return true;
}

bool dsGLRenderer_restoreGlobalState(dsRenderer* renderer)
{
	dsGLRenderer* glRenderer = (dsGLRenderer*)renderer;
	void* context;
	void* surface;
	if (glRenderer->curGLSurface)
	{
		context = glRenderer->renderContext;
		surface = glRenderer->curGLSurface;
	}
	else
	{
		context = glRenderer->sharedContext;
		surface = glRenderer->dummySurface;
	}

	if (!dsBindGLContext(glRenderer->options.display, context, surface))
	{
		errno = EPERM;
		return false;
	}
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
	options->debug = ANYGL_ALLOW_DEBUG;
	options->maxResourceThreads = 0;
	options->shaderCacheDir = NULL;
}

dsRenderer* dsGLRenderer_create(dsAllocator* allocator, const dsOpenGLOptions* options)
{
	if (!allocator || !options)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Renderer allocator must support freeing memory.");
		return NULL;
	}

	if (!AnyGL_initialize())
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Cannot initialize OpenGL.");
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

	size_t bufferSize = dsGLRenderer_fullAllocSize(options);
	void* buffer = dsAllocator_alloc(allocator, bufferSize);
	if (!buffer)
	{
		AnyGL_shutdown();
		return NULL;
	}

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, bufferSize));
	dsGLRenderer* renderer = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAlloc, dsGLRenderer);
	DS_ASSERT(renderer);
	memset(renderer, 0, sizeof(*renderer));
	dsRenderer* baseRenderer = (dsRenderer*)renderer;

	DS_VERIFY(dsRenderer_initialize(baseRenderer));
	baseRenderer->allocator = allocator;
	DS_VERIFY(dsSpinlock_initialize(&renderer->syncPoolLock));
	DS_VERIFY(dsSpinlock_initialize(&renderer->syncRefPoolLock));

	renderer->options = *options;
	if (options->shaderCacheDir)
	{
		size_t length = strlen(options->shaderCacheDir) + 1;
		char* stringCopy = (char*)dsAllocator_alloc((dsAllocator*)&bufferAlloc, length);
		memcpy(stringCopy, options->shaderCacheDir, length);
		renderer->options.shaderCacheDir = stringCopy;
	}

	if (renderer->options.display)
		renderer->releaseDisplay = false;
	else
	{
		renderer->options.display = dsGetGLDisplay();
		renderer->releaseDisplay = true;
	}

	void* display = renderer->options.display;
	renderer->sharedConfig = dsCreateGLConfig(allocator, display, options, false);
	renderer->renderConfig = dsCreateGLConfig(allocator, display, options, true);
	if (!renderer->sharedConfig || !renderer->renderConfig)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Couldn't create OpenGL configuration.");
		dsGLRenderer_destroy(baseRenderer);
		return NULL;
	}

	renderer->dummySurface = dsCreateDummyGLSurface(allocator, display, renderer->sharedConfig,
		&renderer->dummyOsSurface);
	if (!renderer->dummySurface)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Couldn't create dummy OpenGL surface.");
		dsGLRenderer_destroy(baseRenderer);
		return NULL;
	}

	renderer->sharedContext = dsCreateGLContext(allocator, display, renderer->sharedConfig,
		NULL);
	if (!renderer->sharedContext)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Couldn't create OpenGL context.");
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

	int major, minor;
	AnyGL_getGLVersion(&major, &minor, NULL);
	if (!hasRequiredFunctions())
	{
		errno = EPERM;
		DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG, "OpenGL %d.%d is too old.", major, minor);
		dsGLRenderer_destroy(baseRenderer);
		return NULL;
	}

	if (options->debug && ANYGL_ALLOW_DEBUG && ANYGL_SUPPORTED(glDebugMessageCallback))
	{
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(&debugOutput, NULL);
	}

	const char* glslVersion = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
	DS_ASSERT(glslVersion);
	unsigned int glslMajor, glslMinor;
	if (ANYGL_GLES)
		DS_VERIFY(sscanf(glslVersion, "OpenGL ES GLSL ES %u.%u", &glslMajor, &glslMinor) == 2);
	else
		DS_VERIFY(sscanf(glslVersion, "%u.%u", &glslMajor, &glslMinor) == 2);
	renderer->shaderVersion = glslMajor*100 + glslMinor;
	renderer->vendorString = (const char*)glGetString(GL_VENDOR);
	DS_ASSERT(renderer->vendorString);
	renderer->rendererString = (const char*)glGetString(GL_RENDERER);
	DS_ASSERT(renderer->rendererString);

	DS_LOG_DEBUG_F(DS_RENDER_OPENGL_LOG_TAG, "OpenGL%s %u.%u", ANYGL_GLES ? " ES" : "", major,
		minor);
	DS_LOG_DEBUG_F(DS_RENDER_OPENGL_LOG_TAG, "Shader version: %s%u.%u", ANYGL_GLES ? "ES " : "",
		glslMajor, glslMinor);
	DS_LOG_DEBUG_F(DS_RENDER_OPENGL_LOG_TAG, "Vendor: %s", renderer->vendorString);
	DS_LOG_DEBUG_F(DS_RENDER_OPENGL_LOG_TAG, "Renderer: %s", renderer->rendererString);

	// Temporary FBOs used when the shared context
	glGenFramebuffers(1, &renderer->sharedTempFramebuffer);
	glGenFramebuffers(1, &renderer->sharedTempCopyFramebuffer);

	if (ANYGL_SUPPORTED(glDrawBuffers))
		glGetIntegerv(GL_MAX_DRAW_BUFFERS, (GLint*)&baseRenderer->maxColorAttachments);
	else
		baseRenderer->maxColorAttachments = 1;

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

	renderer->contextMutex = dsMutex_create((dsAllocator*)&bufferAlloc, "GL context");
	DS_ASSERT(renderer->contextMutex);
	renderer->curTexture0Target = GL_TEXTURE_2D;
	renderer->curSurfaceType = GLSurfaceType_Left;
	renderer->curFbo = 0;

	baseRenderer->resourceManager = (dsResourceManager*)dsGLResourceManager_create(allocator,
		renderer);
	if (!baseRenderer->resourceManager)
	{
		dsGLRenderer_destroy(baseRenderer);
		return NULL;
	}

	if (ANYGL_GLES)
		baseRenderer->type = DS_GLES_RENDERER_TYPE;
	else
		baseRenderer->type = DS_GL_RENDERER_TYPE;

	switch (ANYGL_LOAD)
	{
		case ANYGL_LOAD_EGL:
			baseRenderer->platformType = DS_EGL_RENDERER_PLATFORM_TYPE;
			break;
		case ANYGL_LOAD_GLX:
			baseRenderer->platformType = DS_GLX_RENDERER_PLATFORM_TYPE;
			break;
		case ANYGL_LOAD_WGL:
			baseRenderer->platformType = DS_WGL_RENDERER_PLATFORM_TYPE;
			break;
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
	baseRenderer->surfaceConfig = dsGetPublicGLConfig(renderer->options.display,
		renderer->renderConfig);
	baseRenderer->surfaceSamples = options->samples;
	baseRenderer->doubleBuffer = options->doubleBuffer;
	baseRenderer->stereoscopic = options->stereoscopic;
	baseRenderer->vsync = false;
	baseRenderer->clipHalfDepth = false;
	baseRenderer->clipInvertY = false;
	baseRenderer->defaultAnisotropy = 1;

	baseRenderer->hasGeometryShaders = (ANYGL_GLES && renderer->shaderVersion >= 320) ||
		(!ANYGL_GLES && renderer->shaderVersion >= 320);
	baseRenderer->hasTessellationShaders = (ANYGL_GLES && renderer->shaderVersion >= 320) ||
		(!ANYGL_GLES && renderer->shaderVersion >= 400);
	baseRenderer->hasComputeShaders = (ANYGL_GLES && renderer->shaderVersion >= 310) ||
		(!ANYGL_GLES && renderer->shaderVersion >= 430);
	baseRenderer->hasNativeMultidraw = ANYGL_SUPPORTED(glMultiDrawArrays);
	baseRenderer->supportsInstancedDrawing = ANYGL_SUPPORTED(glDrawArraysInstanced);
	baseRenderer->supportsStartInstance = ANYGL_SUPPORTED(glDrawArraysInstancedBaseInstance);

	if (AnyGL_EXT_texture_filter_anisotropic)
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &baseRenderer->maxAnisotropy);
	else
		baseRenderer->maxAnisotropy = 1.0f;

	// Render surfaces
	baseRenderer->createRenderSurfaceFunc = &dsGLRenderSurface_create;
	baseRenderer->destroyRenderSurfaceFunc = &dsGLRenderSurface_destroy;
	baseRenderer->updateRenderSurfaceFunc = &dsGLRenderSurface_update;
	baseRenderer->beginRenderSurfaceFunc = &dsGLRenderSurface_beginDraw;
	baseRenderer->endRenderSurfaceFunc = &dsGLRenderSurface_endDraw;
	baseRenderer->swapRenderSurfaceBuffersFunc = &dsGLRenderSurface_swapBuffers;

	// Command buffers
	baseRenderer->createCommandBufferPoolFunc = &dsGLCommandBufferPool_create;
	baseRenderer->destroyCommandBufferPoolFunc = &dsGLCommandBufferPool_destroy;
	baseRenderer->resetCommandBufferPoolFunc = &dsGLCommandBufferPool_reset;
	baseRenderer->beginCommandBufferFunc = &dsGLCommandBuffer_begin;
	baseRenderer->endCommandBufferFunc = &dsGLCommandBuffer_end;
	baseRenderer->submitCommandBufferFunc = &dsGLCommandBuffer_submit;

	// Render passes
	baseRenderer->createRenderPassFunc = &dsGLRenderPass_create;
	baseRenderer->destroyRenderPassFunc = &dsGLRenderPass_destroy;
	baseRenderer->beginRenderPassFunc = &dsGLRenderPass_begin;
	baseRenderer->nextRenderSubpassFunc = &dsGLRenderPass_nextSubpass;
	baseRenderer->endRenderPassFunc = &dsGLRenderPass_end;

	// Renderer functions
	baseRenderer->beginFrameFunc = &dsGLRenderer_beginFrame;
	baseRenderer->endFrameFunc = &dsGLRenderer_endFrame;
	baseRenderer->setSurfaceSamplesFunc = &dsGLRenderer_setSurfaceSamples;
	baseRenderer->setVsyncFunc = &dsGLRenderer_setVsync;
	baseRenderer->setDefaultAnisotropyFunc = &dsGLRenderer_setDefaultAnisotropy;
	baseRenderer->clearColorSurfaceFunc = &dsGLCommandBuffer_clearColorSurface;
	baseRenderer->clearDepthStencilSurfaceFunc = &dsGLCommandBuffer_clearDepthStencilSurface;
	baseRenderer->drawFunc = &dsGLCommandBuffer_draw;
	baseRenderer->drawIndexedFunc = &dsGLCommandBuffer_drawIndexed;
	baseRenderer->drawIndirectFunc = &dsGLCommandBuffer_drawIndirect;
	baseRenderer->drawIndexedIndirectFunc = &dsGLCommandBuffer_drawIndexedIndirect;
	baseRenderer->dispatchComputeFunc = &dsGLCommandBuffer_dispatchCompute;
	baseRenderer->dispatchComputeIndirectFunc = &dsGLCommandBuffer_dispatchComputeIndirect;
	baseRenderer->blitSurfaceFunc = &dsGLCommandBuffer_blitSurface;
	baseRenderer->waitUntilIdleFunc = &dsGLRenderer_waitUntilIdle;
	baseRenderer->restoreGlobalStateFunc = &dsGLRenderer_restoreGlobalState;

	return baseRenderer;
}

void dsGLRenderer_setEnableErrorChecking(dsRenderer* renderer, bool enabled)
{
	if (!renderer)
		return;

	AnyGL_setDebugEnabled(enabled);
}

bool dsGLRenderer_getShaderVersion(uint32_t* outVersion, bool* outGles, const dsRenderer* renderer)
{
	if (!renderer)
	{
		errno = EINVAL;
		return false;
	}

	dsGLRenderer* glRenderer = (dsGLRenderer*)renderer;
	if (outVersion)
		*outVersion = glRenderer->shaderVersion;
	if (outGles)
		*outGles = ANYGL_GLES;
	return true;
}

const char* dsGLRenderer_getVendor(const dsRenderer* renderer)
{
	if (!renderer)
	{
		errno = EINVAL;
		return NULL;
	}

	dsGLRenderer* glRenderer = (dsGLRenderer*)renderer;
	return glRenderer->vendorString;
}

const char* dsGLRenderer_getGLRenderer(const dsRenderer* renderer)
{
	if (!renderer)
	{
		errno = EINVAL;
		return NULL;
	}

	dsGLRenderer* glRenderer = (dsGLRenderer*)renderer;
	return glRenderer->rendererString;
}

bool dsGLRenderer_bindSurface(dsRenderer* renderer, void* glSurface)
{
	dsGLRenderer* glRenderer = (dsGLRenderer*)renderer;
	if (glSurface != glRenderer->curGLSurface)
	{
		if (!dsBindGLContext(glRenderer->options.display, glRenderer->renderContext,
			glSurface))
		{
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Failed to bind render surface. It may have been"
				"destroyed before the commands could execute?");
			return false;
		}
		glRenderer->curGLSurface = glSurface;
		glRenderer->renderContextBound = true;
		if (!glRenderer->renderContextReset)
		{
			glRenderer->renderContextBound = true;
			dsGLMainCommandBuffer_resetState((dsGLMainCommandBuffer*)renderer->mainCommandBuffer);
		}
	}
	// Now that the context is bound, can destroy the deleted objects.
	deleteDestroyedObjects(glRenderer);
	return true;
}

void dsGLRenderer_destroySurface(dsRenderer* renderer, void* glSurface)
{
	dsGLRenderer* glRenderer = (dsGLRenderer*)renderer;
	if (glRenderer->curGLSurface == glSurface)
	{
		DS_VERIFY(dsBindGLContext(glRenderer->options.display, glRenderer->sharedContext,
			glRenderer->dummySurface));
		glRenderer->curGLSurface = NULL;
		glRenderer->renderContextBound = false;
	}
}

void dsGLRenderer_destroyVao(dsRenderer* renderer, GLuint vao, uint32_t contextCount)
{
	if (!vao)
		return;

	dsGLRenderer* glRenderer = (dsGLRenderer*)renderer;
	if (dsThread_equal(dsThread_thisThreadId(), renderer->mainThread) &&
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

	size_t index = glRenderer->curDestroyVaos;
	if (!DS_RESIZEABLE_ARRAY_ADD(renderer->allocator, glRenderer->destroyVaos,
		glRenderer->curDestroyVaos, glRenderer->maxDestroyVaos, 1))
	{
		dsMutex_unlock(glRenderer->contextMutex);
		return;
	}

	DS_ASSERT(index < glRenderer->maxDestroyVaos);
	glRenderer->destroyVaos[index++] = vao;
	dsMutex_unlock(glRenderer->contextMutex);
}

void dsGLRenderer_destroyFbo(dsRenderer* renderer, GLuint fbo, uint32_t contextCount)
{
	if (!fbo)
		return;

	dsGLRenderer* glRenderer = (dsGLRenderer*)renderer;
	if (dsThread_equal(dsThread_thisThreadId(), renderer->mainThread) &&
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

	size_t index = glRenderer->curDestroyFbos;
	if (!DS_RESIZEABLE_ARRAY_ADD(renderer->allocator, glRenderer->destroyFbos,
		glRenderer->curDestroyFbos, glRenderer->maxDestroyFbos, 1))
	{
		dsMutex_unlock(glRenderer->contextMutex);
		return;
	}

	DS_ASSERT(index < glRenderer->maxDestroyFbos);
	glRenderer->destroyFbos[index++] = fbo;
	dsMutex_unlock(glRenderer->contextMutex);
}

void dsGLRenderer_destroyTexture(dsRenderer* renderer, GLuint texture)
{
	dsGLRenderer* glRenderer = (dsGLRenderer*)renderer;

	if (dsThread_equal(dsThread_thisThreadId(), renderer->mainThread) &&
		texture == glRenderer->curTexture0)
	{
		glRenderer->curTexture0 = 0;
	}

	glDeleteTextures(1, &texture);
}

GLuint dsGLRenderer_tempFramebuffer(dsRenderer* renderer)
{
	dsGLRenderer* glRenderer = (dsGLRenderer*)renderer;
	if (!glRenderer->renderContextBound)
		return glRenderer->sharedTempFramebuffer;

	if (glRenderer->tempFramebuffer)
		return glRenderer->tempFramebuffer;

	glGenFramebuffers(1, &glRenderer->tempFramebuffer);
	return glRenderer->tempFramebuffer;
}

GLuint dsGLRenderer_tempCopyFramebuffer(dsRenderer* renderer)
{
	dsGLRenderer* glRenderer = (dsGLRenderer*)renderer;
	if (!glRenderer->renderContextBound)
		return glRenderer->sharedTempCopyFramebuffer;

	if (glRenderer->tempCopyFramebuffer)
		return glRenderer->tempCopyFramebuffer;

	glGenFramebuffers(1, &glRenderer->tempCopyFramebuffer);
	return glRenderer->tempCopyFramebuffer;
}

void dsGLRenderer_restoreFramebuffer(dsRenderer* renderer)
{
	dsGLRenderer* glRenderer = (dsGLRenderer*)renderer;
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, glRenderer->curFbo);
}

dsGLFenceSync* dsGLRenderer_createSync(dsRenderer* renderer, GLsync sync)
{
	dsGLRenderer* glRenderer = (dsGLRenderer*)renderer;
	DS_VERIFY(dsSpinlock_lock(&glRenderer->syncPoolLock));

	int prevErrno = errno;
	dsAllocator* pool = NULL;
	dsGLFenceSync* fenceSync = NULL;
	for (size_t i = 0; i < glRenderer->curSyncPools && !fenceSync; ++i)
	{
		pool = (dsAllocator*)(glRenderer->syncPools + i);
		fenceSync = DS_ALLOCATE_OBJECT(pool, dsGLFenceSync);
		if (fenceSync)
			break;
	}
	errno = prevErrno;

	// All pools are full.
	if (!fenceSync)
	{
		pool = (dsAllocator*)addPool(renderer->allocator, &glRenderer->syncPools,
			&glRenderer->curSyncPools, &glRenderer->maxSyncPools, sizeof(dsGLFenceSync),
			DS_SYNC_POOL_COUNT);
		if (!pool)
		{
			DS_VERIFY(dsSpinlock_unlock(&glRenderer->syncPoolLock));
			return NULL;
		}

		fenceSync = DS_ALLOCATE_OBJECT(pool, dsGLFenceSync);
		DS_ASSERT(fenceSync);
	}
	DS_VERIFY(dsSpinlock_unlock(&glRenderer->syncPoolLock));

	fenceSync->allocator = pool;
	fenceSync->refCount = 1;
	fenceSync->glSync = sync;
	return fenceSync;
}

dsGLFenceSyncRef* dsGLRenderer_createSyncRef(dsRenderer* renderer)
{
	dsGLRenderer* glRenderer = (dsGLRenderer*)renderer;
	DS_VERIFY(dsSpinlock_lock(&glRenderer->syncRefPoolLock));

	int prevErrno = errno;
	dsAllocator* pool = NULL;
	dsGLFenceSyncRef* fenceSyncRef = NULL;
	for (size_t i = 0; i < glRenderer->curSyncRefPools && !fenceSyncRef; ++i)
	{
		pool = (dsAllocator*)(glRenderer->syncRefPools + i);
		fenceSyncRef = DS_ALLOCATE_OBJECT(pool, dsGLFenceSyncRef);
		if (fenceSyncRef)
			break;
	}
	errno = prevErrno;

	// All pools are full.
	if (!fenceSyncRef)
	{
		pool = (dsAllocator*)addPool(renderer->allocator, &glRenderer->syncRefPools,
			&glRenderer->curSyncRefPools, &glRenderer->maxSyncRefPools, sizeof(dsGLFenceSyncRef),
			DS_SYNC_POOL_COUNT);
		if (!pool)
		{
			DS_VERIFY(dsSpinlock_unlock(&glRenderer->syncPoolLock));
			return NULL;
		}

		fenceSyncRef = DS_ALLOCATE_OBJECT(pool, dsGLFenceSyncRef);
		DS_ASSERT(fenceSyncRef);
	}
	DS_VERIFY(dsSpinlock_unlock(&glRenderer->syncRefPoolLock));

	fenceSyncRef->allocator = pool;
	fenceSyncRef->refCount = 1;
	fenceSyncRef->sync = NULL;
	return fenceSyncRef;
}

void dsGLRenderer_bindTexture(dsRenderer* renderer, unsigned int unit, GLenum target,
	GLuint texture)
{
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(target, texture);

	if (unit == 0 && dsThread_equal(dsThread_thisThreadId(), renderer->mainThread))
	{
		dsGLRenderer* glRenderer = (dsGLRenderer*)renderer;
		glRenderer->curTexture0Target = target;
		glRenderer->curTexture0 = texture;
	}
}

void dsGLRenderer_beginTextureOp(dsRenderer* renderer, GLenum target, GLuint texture)
{
	DS_UNUSED(renderer);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(target, texture);
}

void dsGLRenderer_endTextureOp(dsRenderer* renderer)
{
	if (dsThread_equal(dsThread_thisThreadId(), renderer->mainThread))
	{
		dsGLRenderer* glRenderer = (dsGLRenderer*)renderer;
		glBindTexture(glRenderer->curTexture0Target, glRenderer->curTexture0);
	}
	else
		glBindTexture(GL_TEXTURE_2D, 0);
}

void dsGLRenderer_bindFramebuffer(dsRenderer* renderer, GLSurfaceType surfaceType,
	GLuint framebuffer, int flags)
{
	bool draw = (flags & GLFramebufferFlags_Read) == 0;
	GLenum framebufferType = draw ? GL_DRAW_FRAMEBUFFER : GL_READ_FRAMEBUFFER;
	dsGLRenderer* glRenderer = (dsGLRenderer*)renderer;
	if (surfaceType == GLSurfaceType_Framebuffer)
	{
		if (glRenderer->curFbo != framebuffer)
		{
			glBindFramebuffer(framebufferType, framebuffer);
			if (flags == GLFramebufferFlags_Default)
				glRenderer->curFbo = framebuffer;
		}
		glRenderer->curSurfaceType = surfaceType;
	}
	else
	{
		if (glRenderer->curSurfaceType == surfaceType)
			return;

		if (glRenderer->curFbo)
		{
			glBindFramebuffer(framebufferType, 0);
			if (flags == GLFramebufferFlags_Default)
				glRenderer->curFbo = 0;
		}
		glRenderer->curSurfaceType = surfaceType;
		GLenum bufferType;
		if (renderer->stereoscopic)
		{
			if (renderer->doubleBuffer)
			{
				if (surfaceType == GLSurfaceType_Right)
					bufferType = GL_BACK_RIGHT;
				else
					bufferType = GL_BACK_LEFT;
			}
			else
			{
				if (surfaceType == GLSurfaceType_Right)
					bufferType = GL_RIGHT;
				else
					bufferType = GL_LEFT;
			}
		}
		else
		{
			if (renderer->doubleBuffer)
				bufferType = GL_BACK;
			else
				bufferType = GL_FRONT;
		}

		if (draw)
		{
			if (ANYGL_SUPPORTED(glDrawBuffer))
				glDrawBuffer(bufferType);
			else if (ANYGL_SUPPORTED(glDrawBuffers))
				glDrawBuffers(1, &bufferType);
		}
		else if (ANYGL_SUPPORTED(glReadBuffer))
			glReadBuffer(bufferType);
	}
}

void dsGLRenderer_destroy(dsRenderer* renderer)
{
	if (!renderer)
		return;

	dsGLResourceManager_destroy((dsGLResourceManager*)renderer->resourceManager);
	dsGLMainCommandBuffer_destroy((dsGLMainCommandBuffer*)renderer->mainCommandBuffer);

	// Since the context is destroyed, don't worry about deleting any associated OpenGL objects.
	// (especially since some, like FBOs and VAOs, aren't shared across contexts)
	dsGLRenderer* glRenderer = (dsGLRenderer*)renderer;
	void* display = glRenderer->options.display;
	dsDestroyGLContext(display, glRenderer->renderContext);
	dsDestroyGLContext(display, glRenderer->sharedContext);\
	dsDestroyDummyGLSurface(display, glRenderer->dummySurface, glRenderer->dummyOsSurface);
	dsDestroyGLConfig(display, glRenderer->sharedConfig);
	dsDestroyGLConfig(display, glRenderer->renderConfig);

	DS_VERIFY(dsAllocator_free(renderer->allocator, glRenderer->destroyVaos));
	DS_VERIFY(dsAllocator_free(renderer->allocator, glRenderer->destroyFbos));
	dsMutex_destroy(glRenderer->contextMutex);

	if (glRenderer->syncPools)
	{
		for (size_t i = 0; i < glRenderer->curSyncPools; ++i)
			DS_VERIFY(dsAllocator_free(renderer->allocator, glRenderer->syncPools[i].buffer));
		DS_VERIFY(dsAllocator_free(renderer->allocator, glRenderer->syncPools));
	}
	dsSpinlock_shutdown(&glRenderer->syncPoolLock);

	if (glRenderer->syncRefPools)
	{
		for (size_t i = 0; i < glRenderer->curSyncRefPools; ++i)
			DS_VERIFY(dsAllocator_free(renderer->allocator, glRenderer->syncRefPools[i].buffer));
		DS_VERIFY(dsAllocator_free(renderer->allocator, glRenderer->syncRefPools));
	}
	dsSpinlock_shutdown(&glRenderer->syncRefPoolLock);

	if (glRenderer->releaseDisplay)
		dsReleaseGLDisplay(glRenderer->options.display);

	DS_VERIFY(dsAllocator_free(renderer->allocator, renderer));

	AnyGL_shutdown();
}
