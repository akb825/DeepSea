/*
 * Copyright 2017-2018 Aaron Barany
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
#include "Platform/GLPlatform.h"
#include "Resources/GLResourceManager.h"
#include "GLCommandBuffer.h"
#include "GLCommandBufferPool.h"
#include "GLHelpers.h"
#include "GLMainCommandBuffer.h"
#include "GLRenderPass.h"
#include "GLRenderSurface.h"
#include "GLTypes.h"

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
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Renderer.h>
#include <string.h>

#define DS_SYNC_POOL_COUNT 100

static uint32_t initializeCount;

static bool initializeGL()
{
	if (initializeCount > 0)
	{
		++initializeCount;
		return true;
	}

	if (!AnyGL_initialize())
		return false;

	initializeCount = 1;
	return true;
}

static void shutdownGL()
{
	if (initializeCount == 0)
		return;

	if (--initializeCount == 0)
		AnyGL_shutdown();
}

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

static size_t dsGLRenderer_fullAllocSize(const dsRendererOptions* options)
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

static void printGLInfo(dsGLRenderer* renderer, uint32_t major, uint32_t minor, uint32_t glslMajor,
	uint32_t glslMinor)
{
	dsRenderer* baseRenderer = (dsRenderer*)renderer;
	DS_LOG_DEBUG_F(DS_RENDER_OPENGL_LOG_TAG, "OpenGL%s %u.%u", ANYGL_GLES ? " ES" : "", major,
		minor);
	DS_LOG_DEBUG_F(DS_RENDER_OPENGL_LOG_TAG, "Shader version: %s%u.%u", ANYGL_GLES ? "ES " : "",
		glslMajor, glslMinor);
	DS_LOG_DEBUG_F(DS_RENDER_OPENGL_LOG_TAG, "Vendor: %s", baseRenderer->vendorName);
	DS_LOG_DEBUG_F(DS_RENDER_OPENGL_LOG_TAG, "Driver: %s", baseRenderer->deviceName);
	if (ANYGL_SUPPORTED(glGetStringi))
	{
		dsAllocator* allocator = ((dsRenderer*)renderer)->allocator;
		GLint extensionCount = 0;
		glGetIntegerv(GL_NUM_EXTENSIONS, &extensionCount);

		char* buffer = NULL;
		uint32_t bufferSize = 0;
		uint32_t maxBufferSize = 0;
		for (GLint i = 0; i < extensionCount; ++i)
		{
			const char* extension = (const char*)glGetStringi(GL_EXTENSIONS, i);
			size_t length = strlen(extension);
			uint32_t pos;
			uint32_t addLength = (uint32_t)length + 1;
			if (bufferSize == 0)
			{
				pos = 0;
				++addLength;
			}
			else
				pos = bufferSize - 1;

			if (!DS_RESIZEABLE_ARRAY_ADD(allocator, buffer, bufferSize, maxBufferSize, addLength))
			{
				dsAllocator_free(allocator, buffer);
				return;
			}

			buffer[pos++] = ' ';
			DS_ASSERT(bufferSize - pos == length + 1);
			strncpy(buffer + pos, extension, bufferSize - pos);
		}

		DS_LOG_DEBUG_F(DS_RENDER_OPENGL_LOG_TAG, "Extensions:%s", buffer);
		dsAllocator_free(allocator, buffer);
	}
	else
		DS_LOG_DEBUG_F(DS_RENDER_OPENGL_LOG_TAG, "Extensions: %s", glGetString(GL_EXTENSIONS));
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
	if (!DS_RESIZEABLE_ARRAY_ADD(allocator, *pools, *curPools, *maxPools, 1))
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

bool dsGLRenderer_destroy(dsRenderer* renderer)
{
	dsRenderer_shutdownResources(renderer);

	dsGLResourceManager_destroy((dsGLResourceManager*)renderer->resourceManager);
	dsGLMainCommandBuffer_destroy((dsGLMainCommandBuffer*)renderer->mainCommandBuffer);

	// Since the context is destroyed, don't worry about deleting any associated OpenGL objects.
	// (especially since some, like FBOs and VAOs, aren't shared across contexts)
	dsGLRenderer* glRenderer = (dsGLRenderer*)renderer;
	void* display = glRenderer->options.display;
	dsDestroyGLContext(display, glRenderer->renderContext);
	dsDestroyGLContext(display, glRenderer->sharedContext);
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

	shutdownGL();
	return true;
}

void dsGLRenderer_setEnableErrorChecking(dsRenderer* renderer, bool enabled)
{
	DS_UNUSED(renderer);
	AnyGL_setDebugEnabled(enabled);
}

bool dsGLRenderer_beginFrame(dsRenderer* renderer)
{
	dsGLRenderer* glRenderer = (dsGLRenderer*)renderer;
	if (glRenderer->renderContextBound)
		deleteDestroyedObjects(glRenderer);

	return true;
}

bool dsGLRenderer_endFrame(dsRenderer* renderer)
{
	dsGLRenderer* glRenderer = (dsGLRenderer*)renderer;

	if (glRenderer->renderContextBound)
		deleteDestroyedObjects(glRenderer);

	DS_PROFILE_SCOPE_START("glFlush");
	glFlush();
	DS_PROFILE_SCOPE_END();
	return true;
}

bool dsGLRenderer_setSurfaceSamples(dsRenderer* renderer, uint32_t samples)
{
	dsGLRenderer* glRenderer = (dsGLRenderer*)renderer;
	if (samples == renderer->surfaceSamples)
		return true;

	// Need to re-create the render context.
	DS_ASSERT(glRenderer->renderContext);
	DS_ASSERT(glRenderer->renderConfig);

	void* display = glRenderer->options.display;
	dsRendererOptions newOptions = glRenderer->options;
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

	renderer->surfaceConfig = dsGetPublicGLConfig(display, glRenderer->renderConfig);

	// These objects were associated with the now destroyed context.
	clearDestroyedObjects(glRenderer);
	glRenderer->tempFramebuffer = 0;
	glRenderer->tempCopyFramebuffer = 0;
	memset(glRenderer->boundAttributes, 0, sizeof(glRenderer->boundAttributes));

	renderer->surfaceSamples = samples;

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

bool dsGLRenderer_flush(dsRenderer* renderer)
{
	DS_UNUSED(renderer);
	glFlush();
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

bool dsGLRenderer_isSupported(void)
{
	bool supported = initializeGL();
	shutdownGL();
	return supported;
}

bool dsGLRenderer_queryDevices(dsRenderDeviceInfo* outDevices, uint32_t* outDeviceCount)
{
	DS_UNUSED(outDevices);
	if (!outDeviceCount)
	{
		errno = EINVAL;
		return false;
	}

	*outDeviceCount = 0;
	return true;
}

dsRenderer* dsGLRenderer_create(dsAllocator* allocator, const dsRendererOptions* options)
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

	dsGfxFormat colorFormat = dsRenderer_optionsColorFormat(options, false, false);
	if (!dsGfxFormat_isValid(colorFormat))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Invalid color format.");
		return NULL;
	}

	dsGfxFormat depthFormat = dsRenderer_optionsDepthFormat(options);

	if (!initializeGL())
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Cannot initialize OpenGL.");
		return NULL;
	}

	size_t bufferSize = dsGLRenderer_fullAllocSize(options);
	void* buffer = dsAllocator_alloc(allocator, bufferSize);
	if (!buffer)
	{
		AnyGL_shutdown();
		return NULL;
	}

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, bufferSize));
	dsGLRenderer* renderer = DS_ALLOCATE_OBJECT(&bufferAlloc, dsGLRenderer);
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
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Couldn't create OpenGL configuration.");
		dsGLRenderer_destroy(baseRenderer);
		// Set errno after destroy so it doesn't get changed.
		errno = EPERM;
		return NULL;
	}

	renderer->dummySurface = dsCreateDummyGLSurface(allocator, display, renderer->sharedConfig,
		&renderer->dummyOsSurface);
	if (!renderer->dummySurface)
	{
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Couldn't create dummy OpenGL surface.");
		dsGLRenderer_destroy(baseRenderer);
		errno = EPERM;
		return NULL;
	}

	renderer->sharedContext = dsCreateGLContext(allocator, display, renderer->sharedConfig,
		NULL);
	if (!renderer->sharedContext)
	{
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Couldn't create OpenGL context.");
		dsGLRenderer_destroy(baseRenderer);
		errno = EPERM;
		return NULL;
	}

	if (!dsBindGLContext(display, renderer->sharedContext, renderer->dummySurface))
	{
		dsGLRenderer_destroy(baseRenderer);
		errno = EPERM;
		return NULL;
	}

	if (!AnyGL_load())
	{
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Couldn't load GL functions.");
		dsGLRenderer_destroy(baseRenderer);
		errno = EPERM;
		return NULL;
	}

	int major, minor;
	AnyGL_getGLVersion(&major, &minor, NULL);
	if (!hasRequiredFunctions())
	{
		DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG, "OpenGL %d.%d is too old.", major, minor);
		dsGLRenderer_destroy(baseRenderer);
		errno = EPERM;
		return NULL;
	}

	if (options->debug && ANYGL_SUPPORTED(glDebugMessageCallback))
	{
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(&debugOutput, NULL);
	}

	const char* glslVersion = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
	DS_ASSERT(glslVersion);
	unsigned int glslMajor, glslMinor;
	if (ANYGL_GLES)
	{
		baseRenderer->name = "OpenGL ES";
		baseRenderer->shaderLanguage = "glsl-es";
		DS_VERIFY(sscanf(glslVersion, "OpenGL ES GLSL ES %u.%u", &glslMajor, &glslMinor) == 2);
	}
	else
	{
		baseRenderer->name = "OpenGL";
		baseRenderer->shaderLanguage = "glsl";
		DS_VERIFY(sscanf(glslVersion, "%u.%u", &glslMajor, &glslMinor) == 2);
	}
	glslMinor /= 10;
	baseRenderer->shaderVersion = DS_ENCODE_VERSION(glslMajor, glslMinor, 0);
	baseRenderer->vendorName = (const char*)glGetString(GL_VENDOR);
	DS_ASSERT(baseRenderer->vendorName);
	baseRenderer->deviceName = (const char*)glGetString(GL_RENDERER);
	DS_ASSERT(baseRenderer->deviceName);

	printGLInfo(renderer, major, minor, glslMajor, glslMinor);

	// Temporary FBOs used when the shared context
	glGenFramebuffers(1, &renderer->sharedTempFramebuffer);
	glGenFramebuffers(1, &renderer->sharedTempCopyFramebuffer);

	if (ANYGL_SUPPORTED(glDrawBuffers))
	{
		glGetIntegerv(GL_MAX_DRAW_BUFFERS, (GLint*)&baseRenderer->maxColorAttachments);
		baseRenderer->maxColorAttachments = dsMin(baseRenderer->maxColorAttachments,
			DS_MAX_ATTACHMENTS);
	}
	else
		baseRenderer->maxColorAttachments = 1;

	GLint maxSamples = 0;
	glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
	maxSamples = dsMax(1, maxSamples);
	baseRenderer->maxSurfaceSamples = dsMin(maxSamples, DS_MAX_ANTIALIAS_SAMPLES);
	renderer->options.samples = dsMin(renderer->options.samples, (uint8_t)maxSamples);

	renderer->renderContext = dsCreateGLContext(allocator, display, renderer->renderConfig,
		renderer->sharedContext);
	if (!renderer->renderContext)
	{
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Couldn't create GL context.");
		dsGLRenderer_destroy(baseRenderer);
		errno = EPERM;
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

	baseRenderer->platform = options->platform;
	if (ANYGL_GLES)
		baseRenderer->rendererID = DS_GLES_RENDERER_ID;
	else
		baseRenderer->rendererID = DS_GL_RENDERER_ID;

	switch (ANYGL_LOAD)
	{
		case ANYGL_LOAD_EGL:
			baseRenderer->platformID = DS_EGL_RENDERER_PLATFORM_ID;
			break;
		case ANYGL_LOAD_GLX:
			baseRenderer->platformID = DS_GLX_RENDERER_PLATFORM_ID;
			break;
		case ANYGL_LOAD_WGL:
			baseRenderer->platformID = DS_WGL_RENDERER_PLATFORM_ID;
			break;
		default:
			baseRenderer->platformID = 0;
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
	baseRenderer->clipHalfDepth = options->preferHalfDepthRange && ANYGL_SUPPORTED(glClipControl);
	baseRenderer->clipInvertY = false;
	baseRenderer->defaultAnisotropy = 1;

	baseRenderer->hasGeometryShaders =
		(ANYGL_GLES && baseRenderer->shaderVersion >= DS_ENCODE_VERSION(3, 2, 0)) ||
		(!ANYGL_GLES && baseRenderer->shaderVersion >= DS_ENCODE_VERSION(3, 2, 0));
	baseRenderer->hasTessellationShaders =
		(ANYGL_GLES && baseRenderer->shaderVersion >= DS_ENCODE_VERSION(3, 2, 0)) ||
		(!ANYGL_GLES && baseRenderer->shaderVersion >= DS_ENCODE_VERSION(4, 0, 0));
	if ((ANYGL_GLES && baseRenderer->shaderVersion >= DS_ENCODE_VERSION(3, 1, 0)) ||
		(!ANYGL_GLES && baseRenderer->shaderVersion >= DS_ENCODE_VERSION(4, 3, 0)))
	{
		for (int i = 0; i < 3; ++i)
		{
			glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, i,
				(GLint*)(baseRenderer->maxComputeWorkGroupSize + i));
		}
	}
	baseRenderer->hasNativeMultidraw = ANYGL_SUPPORTED(glMultiDrawArrays);
	baseRenderer->hasInstancedDrawing = ANYGL_SUPPORTED(glDrawArraysInstanced);
	baseRenderer->hasStartInstance = ANYGL_SUPPORTED(glDrawArraysInstancedBaseInstance);
	baseRenderer->hasIndependentBlend = ANYGL_SUPPORTED(glBlendFunci);
	baseRenderer->hasDualSrcBlend = AnyGL_atLeastVersion(3, 3, false) ||
		AnyGL_ARB_blend_func_extended || AnyGL_EXT_blend_func_extended;
	baseRenderer->hasLogicOps = ANYGL_SUPPORTED(glLogicOp);
	baseRenderer->hasSampleShading = ANYGL_SUPPORTED(glMinSampleShading);
	baseRenderer->hasDepthBounds = AnyGL_EXT_depth_bounds_test;
	baseRenderer->hasDepthClamp = (AnyGL_atLeastVersion(3, 2, false) || AnyGL_ARB_depth_clamp);
	baseRenderer->hasDepthBiasClamp = ANYGL_SUPPORTED(glPolygonOffsetClamp);

	if (AnyGL_EXT_texture_filter_anisotropic)
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &baseRenderer->maxAnisotropy);
	else
		baseRenderer->maxAnisotropy = 1.0f;

	baseRenderer->destroyFunc = &dsGLRenderer_destroy;
	baseRenderer->setExtraDebuggingFunc = &dsGLRenderer_setEnableErrorChecking;

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
	baseRenderer->beginSecondaryCommandBufferFunc = &dsGLCommandBuffer_beginSecondary;
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
	baseRenderer->pushDebugGroupFunc = &dsGLCommandBuffer_pushDebugGroup;
	baseRenderer->popDebugGroupFunc = &dsGLCommandBuffer_popDebugGroup;
	baseRenderer->memoryBarrierFunc = &dsGLCommandBuffer_memoryBarrier;
	baseRenderer->flushFunc = &dsGLRenderer_flush;
	baseRenderer->waitUntilIdleFunc = &dsGLRenderer_waitUntilIdle;
	baseRenderer->restoreGlobalStateFunc = &dsGLRenderer_restoreGlobalState;

	DS_VERIFY(dsRenderer_initializeResources(baseRenderer));

	return baseRenderer;
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
			glRenderer->renderContextReset = true;
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
	if (dsThread_equal(dsThread_thisThreadID(), renderer->mainThread) &&
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
	if (dsThread_equal(dsThread_thisThreadID(), renderer->mainThread) &&
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

	if (dsThread_equal(dsThread_thisThreadID(), renderer->mainThread) &&
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

	if (unit == 0 && dsThread_equal(dsThread_thisThreadID(), renderer->mainThread))
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
	if (dsThread_equal(dsThread_thisThreadID(), renderer->mainThread))
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
