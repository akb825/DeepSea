/*
 * Copyright 2018-2021 Aaron Barany
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

#include <DeepSea/RenderBootstrap/RenderBootstrap.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#if DS_HAS_RENDER_OPENGL
#include <DeepSea/RenderOpenGL/GLRenderer.h>
#endif

#if DS_HAS_RENDER_VULKAN
#include <DeepSea/RenderVulkan/VkRenderer.h>
#endif

#if DS_HAS_RENDER_METAL
#include <DeepSea/RenderMetal/MTLRenderer.h>
#endif

#include <string.h>

#if DS_WINDOWS
#define strcasecmp(x, y) _stricmp(x, y)
#else
#include <strings.h>
#endif

static const char* rendererNames[] =
{
	"Metal",
	"Vulkan",
	"OpenGL"
};

#if DS_HAS_RENDER_VULKAN
// Drivers that have significant issues observed in Vulkan, so blacklist them from default driver
// detection.
static const char* blacklistedVulkanDrivers[] =
{
	"none" // Satisfy MSVC
};
#endif

_Static_assert(DS_ARRAY_SIZE(rendererNames) == (uint32_t)dsRendererType_Default,
	"Unexpected length of renderNames.");

static dsRendererType defaultRenderer = dsRendererType_Default;

const char* dsRenderBootstrap_rendererName(dsRendererType type)
{
	if (type == dsRendererType_Default)
		type = dsRenderBootstrap_defaultRenderer();

	if ((int)type < 0 || type >= dsRendererType_Default)
		return NULL;
	return rendererNames[type];
}

dsRendererType dsRenderBootstrap_rendererTypeFromName(const char* name)
{
	if (!name)
		return dsRendererType_Default;

	for (int i = 0; i < dsRendererType_Default; ++i)
	{
		if (strcasecmp(name, rendererNames[i]) == 0)
			return (dsRendererType)i;
	}
	return dsRendererType_Default;
}

uint32_t dsRenderBootstrap_rendererID(dsRendererType type)
{
	if (type == dsRendererType_Default)
		type = dsRenderBootstrap_defaultRenderer();

	switch (type)
	{
#if DS_HAS_RENDER_METAL
		case dsRendererType_Metal:
			return DS_MTL_RENDERER_ID;
#endif
#if DS_HAS_RENDER_VULKAN
		case dsRendererType_Vulkan:
			return DS_VK_RENDERER_ID;
#endif
#if DS_HAS_RENDER_OPENGL
		case dsRendererType_OpenGL:
			if (dsGLRenderer_isGLES)
				return DS_GLES_RENDERER_ID;
			return DS_GL_RENDERER_ID;
#endif
		default:
			return 0;
	}
}

dsRendererType dsRenderBootstrap_defaultRenderer(void)
{
	if (defaultRenderer != dsRendererType_Default)
		return defaultRenderer;

#if DS_HAS_RENDER_METAL
	if (dsMTLRenderer_isSupported())
	{
		defaultRenderer = dsRendererType_Metal;
		return defaultRenderer;
	}
#endif

#if DS_HAS_RENDER_VULKAN
	if (dsVkRenderer_isSupported())
	{
		dsRenderDeviceInfo defaultDevice;
		DS_VERIFY(dsVkRenderer_getDefaultDevice(&defaultDevice));
		bool blacklisted = false;
		for (uint32_t i = 0; i < DS_ARRAY_SIZE(blacklistedVulkanDrivers); ++i)
		{
			if (strstr(defaultDevice.name, blacklistedVulkanDrivers[i]))
			{
				blacklisted = true;
				DS_LOG_INFO_F(DS_RENDER_BOOTSTRAP_LOG_TAG,
					"Vulkan renderer disabled by default for device %s", defaultDevice.name);
				break;
			}
		}

		if (!blacklisted)
		{
			defaultRenderer = dsRendererType_Vulkan;
			return defaultRenderer;
		}
	}
#endif

#if DS_HAS_RENDER_OPENGL
	if (dsGLRenderer_isSupported())
	{
		defaultRenderer = dsRendererType_OpenGL;
		return defaultRenderer;
	}
#endif

	return defaultRenderer;
}

bool dsRenderBootstrap_isSupported(dsRendererType type)
{
	if (type == dsRendererType_Default)
		return dsRenderBootstrap_defaultRenderer() != dsRendererType_Default;

	switch (type)
	{
		case dsRendererType_Metal:
#if DS_HAS_RENDER_METAL
			return dsMTLRenderer_isSupported();
#else
			return false;
#endif
		case dsRendererType_Vulkan:
#if DS_HAS_RENDER_VULKAN
			return dsVkRenderer_isSupported();
#else
			return false;
#endif
		case dsRendererType_OpenGL:
#if DS_HAS_RENDER_OPENGL
			return dsGLRenderer_isSupported();
#else
			return false;
#endif
		default:
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_BOOTSTRAP_LOG_TAG, "Unknown renderer type.");
			return false;
	}
}

bool dsRenderBootstrap_queryDevices(dsRenderDeviceInfo* outDevices, uint32_t* outDeviceCount,
	dsRendererType type)
{
	if (type == dsRendererType_Default)
		type = dsRenderBootstrap_defaultRenderer();

	switch (type)
	{
		case dsRendererType_Metal:
#if DS_HAS_RENDER_METAL
			return dsMTLRenderer_queryDevices(outDevices, outDeviceCount);
#else
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_BOOTSTRAP_LOG_TAG,
				"Metal renderer not supported on this platform.");
			return false;
#endif
		case dsRendererType_Vulkan:
#if DS_HAS_RENDER_VULKAN
			return dsVkRenderer_queryDevices(outDevices, outDeviceCount);
#else
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_BOOTSTRAP_LOG_TAG,
				"Vulkan renderer not supported on this platform.");
			return false;
#endif
		case dsRendererType_OpenGL:
#if DS_HAS_RENDER_OPENGL
			return dsGLRenderer_queryDevices(outDevices, outDeviceCount);
#else
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_BOOTSTRAP_LOG_TAG,
				"OpenGL renderer not supported on this platform.");
			return false;
#endif
		default:
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_BOOTSTRAP_LOG_TAG, "Unknown renderer type.");
			return false;
	}
}

dsRenderer* dsRenderBootstrap_createRenderer(dsRendererType type, dsAllocator* allocator,
	const dsRendererOptions* options)
{
	if (type == dsRendererType_Default)
		type = dsRenderBootstrap_defaultRenderer();

	switch (type)
	{
		case dsRendererType_Metal:
		{
#if DS_HAS_RENDER_METAL
			return dsMTLRenderer_create(allocator, options);
#else
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_BOOTSTRAP_LOG_TAG,
				"Metal renderer not supported on this platform.");
			return NULL;
#endif
		}
		case dsRendererType_Vulkan:
		{
#if DS_HAS_RENDER_VULKAN
			return dsVkRenderer_create(allocator, options);
#else
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_BOOTSTRAP_LOG_TAG,
				"Vulkan renderer not supported on this platform.");
			return NULL;
#endif
		}
		case dsRendererType_OpenGL:
		{
#if DS_HAS_RENDER_OPENGL
			dsRenderer* renderer = dsGLRenderer_create(allocator, options);
			if (!renderer && errno == EPERM && options->surfaceSamples > 1)
			{
				DS_LOG_INFO(DS_RENDER_BOOTSTRAP_LOG_TAG,
					"Failed creating OpenGL renderer. Trying again without anti-aliasing.");
				dsRendererOptions altOptions = *options;
				altOptions.surfaceSamples = 1;
				renderer = dsGLRenderer_create(allocator, &altOptions);
			}
			return renderer;
#else
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_BOOTSTRAP_LOG_TAG,
				"OpenGL renderer not supported on this platform.");
			return NULL;
#endif
		}
		default:
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_BOOTSTRAP_LOG_TAG, "Unknown renderer type.");
			return NULL;
	}
}
