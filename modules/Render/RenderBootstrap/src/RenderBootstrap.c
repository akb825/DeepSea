/*
 * Copyright 2018 Aaron Barany
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

DS_STATIC_ASSERT(DS_ARRAY_SIZE(rendererNames) == (uint32_t)dsRendererType_Default,
	unexpected_names_size);

static dsRenderer* createRendererImpl(dsRendererType type, dsAllocator* allocator,
	const dsRendererOptions* options, bool handleError)
{
	switch (type)
	{
		case dsRendererType_Metal:
		{
#if DS_HAS_RENDER_METAL
			if (handleError)
			{
				errno = EPERM;
				DS_LOG_ERROR(DS_RENDER_BOOTSTRAP_LOG_TAG,
					"Metal renderer not yet implemented.");
			}
			return NULL;
#else
			if (handleError)
			{
				errno = EPERM;
				DS_LOG_ERROR(DS_RENDER_BOOTSTRAP_LOG_TAG,
					"Metal renderer not supported on this platform.");
			}
			return NULL;
#endif
		}
		case dsRendererType_Vulkan:
		{
#if DS_HAS_RENDER_VULKAN
			return NULL;
#else
			if (handleError)
			{
				errno = EPERM;
				DS_LOG_ERROR(DS_RENDER_BOOTSTRAP_LOG_TAG,
					"Vulkan renderer not supported on this platform.");
			}
			return NULL;
#endif
		}
		case dsRendererType_OpenGL:
		{
#if DS_HAS_RENDER_OPENGL
			dsRenderer* renderer = dsGLRenderer_create(allocator, options);
			if (!renderer && errno == EPERM && options->samples > 1)
			{
				DS_LOG_INFO(DS_RENDER_BOOTSTRAP_LOG_TAG,
					"Failed creating OpenGL renderer. Trying again without anti-aliasing.");
				dsRendererOptions altOptions = *options;
				altOptions.samples = 1;
				renderer = dsGLRenderer_create(allocator, &altOptions);
			}
			return renderer;
#else
			if (handleError)
			{
				errno = EPERM;
				DS_LOG_ERROR(DS_RENDER_BOOTSTRAP_LOG_TAG,
					"OpenGL renderer not supported on this platform.");
			}
			return NULL;
#endif
		}
		default:
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_BOOTSTRAP_LOG_TAG, "Unknown renderer type.");
			return NULL;
	}
}

const char* dsRenderBootstrap_rendererName(dsRendererType type)
{
	if (type < 0 || type >= dsRendererType_Default)
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

dsRenderer* dsRenderBootstrap_createRenderer(dsRendererType type, dsAllocator* allocator,
	const dsRendererOptions* options)
{
	if (type == dsRendererType_Default)
	{
		for (int i = 0; i < dsRendererType_Default; ++i)
		{
			dsRenderer* renderer = createRendererImpl((dsRendererType)i, allocator, options, false);
			if (renderer)
				return renderer;
		}
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_BOOTSTRAP_LOG_TAG, "No suitable renderer found.");
		return NULL;
	}

	return createRendererImpl(type, allocator, options, true);
}
