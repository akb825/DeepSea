/*
 * Copyright 2025 Aaron Barany
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

#include "Platform/GLPlatform.h"
#include "Platform/GLPlatformCocoa.h"
#include "Platform/GLPlatformEGL.h"
#include "Platform/GLPlatformGLX.h"
#include "Platform/GLPlatformWGL.h"

#include <DeepSea/Core/Assert.h>

bool dsGLPlatform_initialize(dsGLPlatform* platform, int anyglLoad)
{
	switch (anyglLoad)
	{
#if DS_MAC
		case ANYGL_LOAD_FPTR:
			platform->getDisplayFunc = &dsGetCocoaGLDisplay;
			platform->releaseDisplayFunc = &dsReleaseCocoaGLDisplay;
			platform->createConfigFunc = &dsCreateCocoaGLConfig;
			platform->getPublicConfigFunc = &dsGetPublicCocoaGLConfig;
			platform->destroyConfigFunc = &dsDestroyCocoaGLConfig;
			platform->createContextFunc = &dsCreateCocoaGLContext;
			platform->destroyContextFunc = &dsDestroyCocoaGLContext;
			platform->createDummySurfaceFunc = &dsCreateDummyCocoaGLSurface;
			platform->destroyDummySurfaceFunc = &dsDestroyDummyCocoaGLSurface;
			platform->createSurfaceFunc = &dsCreateCocoaGLSurface;
			platform->getSurfaceSizeFunc = &dsGetCocoaGLSurfaceSize;
			platform->swapBuffersFunc = &dsSwapCocoaGLBuffers;
			platform->destroySurfaceFunc = &dsDestroyCocoaGLSurface;
			platform->bindContextFunc = &dsBindCocoaGLContext;
			platform->getCurrentContextFunc = &dsGetCurrentCocoaGLContext;
			platform->setVSyncFunc = &dsSetCocoaGLVSync;
			return true;
#endif
#if ANYGL_HAS_EGL
		case ANYGL_LOAD_EGL:
			platform->getDisplayFunc = &dsGetEGLDisplay;
			platform->releaseDisplayFunc = &dsReleaseEGLDisplay;
			platform->createConfigFunc = &dsCreateEGLConfig;
			platform->getPublicConfigFunc = &dsGetPublicEGLConfig;
			platform->destroyConfigFunc = &dsDestroyEGLConfig;
			platform->createContextFunc = &dsCreateEGLContext;
			platform->destroyContextFunc = &dsDestroyEGLContext;
			platform->createDummySurfaceFunc = &dsCreateDummyEGLSurface;
			platform->destroyDummySurfaceFunc = &dsDestroyDummyEGLSurface;
			platform->createSurfaceFunc = &dsCreateEGLSurface;
			platform->getSurfaceSizeFunc = &dsGetEGLSurfaceSize;
			platform->swapBuffersFunc = &dsSwapEGLBuffers;
			platform->destroySurfaceFunc = &dsDestroyEGLSurface;
			platform->bindContextFunc = &dsBindEGLContext;
			platform->getCurrentContextFunc = &dsGetCurrentEGLContext;
			platform->setVSyncFunc = &dsSetEGLVSync;
			return true;
#endif
#if ANYGL_HAS_GLX
		case ANYGL_LOAD_GLX:
			platform->getDisplayFunc = &dsGetGLXDisplay;
			platform->releaseDisplayFunc = &dsReleaseGLXDisplay;
			platform->createConfigFunc = &dsCreateGLXConfig;
			platform->getPublicConfigFunc = &dsGetPublicGLXConfig;
			platform->destroyConfigFunc = &dsDestroyGLXConfig;
			platform->createContextFunc = &dsCreateGLXContext;
			platform->destroyContextFunc = &dsDestroyGLXContext;
			platform->createDummySurfaceFunc = &dsCreateDummyGLXSurface;
			platform->destroyDummySurfaceFunc = &dsDestroyDummyGLXSurface;
			platform->createSurfaceFunc = &dsCreateGLXSurface;
			platform->getSurfaceSizeFunc = &dsGetGLXSurfaceSize;
			platform->swapBuffersFunc = &dsSwapGLXBuffers;
			platform->destroySurfaceFunc = &dsDestroyGLXSurface;
			platform->bindContextFunc = &dsBindGLXContext;
			platform->getCurrentContextFunc = &dsGetCurrentGLXContext;
			platform->setVSyncFunc = &dsSetGLXVSync;
			return true;
#endif
#if ANYGL_HAS_WGL
		case ANYGL_LOAD_WGL:
			platform->getDisplayFunc = &dsGetWGLDisplay;
			platform->releaseDisplayFunc = &dsReleaseWGLDisplay;
			platform->createConfigFunc = &dsCreateWGLConfig;
			platform->getPublicConfigFunc = &dsGetPublicWGLConfig;
			platform->destroyConfigFunc = &dsDestroyWGLConfig;
			platform->createContextFunc = &dsCreateWGLContext;
			platform->destroyContextFunc = &dsDestroyWGLContext;
			platform->createDummySurfaceFunc = &dsCreateDummyWGLSurface;
			platform->destroyDummySurfaceFunc = &dsDestroyDummyWGLSurface;
			platform->createSurfaceFunc = &dsCreateWGLSurface;
			platform->getSurfaceSizeFunc = &dsGetWGLSurfaceSize;
			platform->swapBuffersFunc = &dsSwapWGLBuffers;
			platform->destroySurfaceFunc = &dsDestroyWGLSurface;
			platform->bindContextFunc = &dsBindWGLContext;
			platform->getCurrentContextFunc = &dsGetCurrentWGLContext;
			platform->setVSyncFunc = &dsSetWGLVSync;
			return true;
#endif
	}
	return false;
}

void* dsGLPlatform_getDisplay(const dsGLPlatform* platform, void* osDisplay)
{
	return platform->getDisplayFunc(osDisplay);
}

void dsGLPlatform_releaseDisplay(const dsGLPlatform* platform, void* osDisplay, void* gfxDisplay)
{
	platform->releaseDisplayFunc(osDisplay, gfxDisplay);
}

void* dsGLPlatform_createConfig(const dsGLPlatform* platform, dsAllocator* allocator, void* display,
	const dsRendererOptions* options, bool render)
{
	GLContextType contextType;
	if (render)
		contextType = GLContextType_Render;
	else if (options->createBackgroundSurfaceFunc && options->destroyBackgroundSurfaceFunc)
		contextType = GLContextType_SharedBackgroundSurface;
	else
		contextType = GLContextType_SharedDummySurface;
	return platform->createConfigFunc(allocator, display, options, contextType);
}

void* dsGLPlatform_getPublicConfig(const dsGLPlatform* platform, void* display, void* config)
{
	return platform->getPublicConfigFunc(display, config);
}

void dsGLPlatform_destroyConfig(const dsGLPlatform* platform, void* display, void* config)
{
	platform->destroyConfigFunc(display, config);
}

void* dsGLPlatform_createContext(const dsGLPlatform* platform, dsAllocator* allocator,
	void* display, void* config, void* shareContext)
{
	return platform->createContextFunc(allocator, display, config, shareContext);
}

void dsGLPlatform_destroyContext(const dsGLPlatform* platform, void* display, void* context)
{
	platform->destroyContextFunc(display, context);
}

void* dsGLPlatform_createDummySurface(const dsGLPlatform* platform, dsAllocator* allocator,
	void* display, const dsRendererOptions* options, void* config, void** osSurface)
{
	if (options->createBackgroundSurfaceFunc && options->destroyBackgroundSurfaceFunc)
	{
		*osSurface = options->createBackgroundSurfaceFunc(
			options->backgroundSurfaceUserData, options->backgroundSurfaceType);
		if (!*osSurface)
			return NULL;

		void* surfaceHandle = *osSurface;
		if (options->getBackgroundSurfaceHandleFunc)
		{
			surfaceHandle = options->getBackgroundSurfaceHandleFunc(
				options->backgroundSurfaceUserData, options->backgroundSurfaceType, *osSurface);
		}
		return platform->createSurfaceFunc(
			allocator, display, config, options->backgroundSurfaceType, surfaceHandle);
	}

	return platform->createDummySurfaceFunc(allocator, display, config, osSurface);
}

void dsGLPlatform_destroyDummySurface( const dsGLPlatform* platform, void* display,
	const dsRendererOptions* options, void* surface, void* osSurface)
{
	if (options->createBackgroundSurfaceFunc && options->destroyBackgroundSurfaceFunc)
	{
		platform->destroySurfaceFunc(display, options->backgroundSurfaceType, surface);
		options->destroyBackgroundSurfaceFunc(
			options->backgroundSurfaceUserData, options->backgroundSurfaceType, osSurface);
	}
	else
		platform->destroyDummySurfaceFunc(display, surface, osSurface);
}

void* dsGLPlatform_createSurface(const dsGLPlatform* platform, dsAllocator* allocator,
	void* display, void* config, dsRenderSurfaceType surfaceType, void* handle)
{
	return platform->createSurfaceFunc(allocator, display, config, surfaceType, handle);
}

bool dsGLPlatform_getSurfaceSize(uint32_t* outWidth, uint32_t* outHeight,
	const dsGLPlatform* platform, void* display, dsRenderSurfaceType surfaceType, void* surface)
{
	return platform->getSurfaceSizeFunc(outWidth, outHeight, display, surfaceType, surface);
}

void dsGLPlatform_swapBuffers(const dsGLPlatform* platform, void* display,
	dsRenderSurface** renderSurfaces, uint32_t count, bool vsync)
{
	platform->swapBuffersFunc(display, renderSurfaces, count, vsync);
}

void dsGLPlatform_destroySurface(
	const dsGLPlatform* platform, void* display, dsRenderSurfaceType surfaceType, void* surface)
{
	platform->destroySurfaceFunc(display, surfaceType, surface);
}

bool dsGLPlatform_bindContext(
	const dsGLPlatform* platform, void* display, void* context, void* surface)
{
	return platform->bindContextFunc(display, context, surface);
}

void* dsGLPlatform_getCurrentContext(const dsGLPlatform* platform, void* display)
{
	return platform->getCurrentContextFunc(display);
}

void dsGLPlatform_setVSync(const dsGLPlatform* platform, void* display, void* surface, bool vsync)
{
	platform->setVSyncFunc(display, surface, vsync);
}
