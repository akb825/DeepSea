/*
 * Copyright 2017-2026 Aaron Barany
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

#include <DeepSea/Render/RenderSurface.h>

#include "GPUProfileContext.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Thread/Thread.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>

#include <DeepSea/Math/Matrix22.h>
#include <DeepSea/Math/Matrix44.h>

#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/RenderSurfaceHint.h>

#include <stdio.h>

#define SCOPE_SIZE 256

static void beginSurfaceScope(const dsRenderSurface* renderSurface)
{
#if DS_PROFILING_ENABLED
	if (renderSurface)
	{
		char buffer[SCOPE_SIZE];
		int result = snprintf(buffer, SCOPE_SIZE, "Surface: %s", renderSurface->name);
		DS_UNUSED(result);
		DS_ASSERT(result > 0 && result < SCOPE_SIZE);
		DS_PROFILE_DYNAMIC_SCOPE_START(buffer);
	}
#else
	DS_UNUSED(renderSurface);
#endif
}

static void endSurfaceScope(const dsRenderSurface* renderSurface)
{
#if DS_PROFILING_ENABLED
	if (renderSurface)
	{
		DS_PROFILE_SCOPE_END();
	}
#else
	DS_UNUSED(renderSurface);
#endif
}

bool dsRenderSurface_makeRotationMatrix22(dsMatrix22f* result, dsRenderSurfaceRotation rotation)
{
	if (!result)
	{
		errno = EINVAL;
		return false;
	}

	switch (rotation)
	{
		case dsRenderSurfaceRotation_0:
			dsMatrix22_identity(*result);
			return true;
		case dsRenderSurfaceRotation_90:
			dsMatrix22_identity(*result);
			result->columns[0].x = 0.0f;
			result->columns[0].y = 1.0f;
			result->columns[1].x = -1.0f;
			result->columns[1].y = 0.0f;
			return true;
		case dsRenderSurfaceRotation_180:
			dsMatrix22_identity(*result);
			result->columns[0].x = -1.0f;
			result->columns[0].y = 0.0f;
			result->columns[1].x = 0.0f;
			result->columns[1].y = -1.0f;
			return true;
		case dsRenderSurfaceRotation_270:
			dsMatrix22_identity(*result);
			result->columns[0].x = 0.0f;
			result->columns[0].y = -1.0f;
			result->columns[1].x = 1.0f;
			result->columns[1].y = 0.0f;
			return true;
		default:
			errno = EINVAL;
			return false;
	}
}

bool dsRenderSurface_makeRotationMatrix44(dsMatrix44f* result, dsRenderSurfaceRotation rotation)
{
	if (!result)
	{
		errno = EINVAL;
		return false;
	}

	switch (rotation)
	{
		case dsRenderSurfaceRotation_0:
			dsMatrix44f_identity(result);
			return true;
		case dsRenderSurfaceRotation_90:
			result->columns[0].x = 0.0f;
			result->columns[0].y = 1.0f;
			result->columns[0].z = 0.0f;
			result->columns[0].w = 0.0f;
			result->columns[1].x = -1.0f;
			result->columns[1].y = 0.0f;
			result->columns[1].z = 0.0f;
			result->columns[1].w = 0.0f;
			result->columns[2].x = 0.0f;
			result->columns[2].y = 0.0f;
			result->columns[2].z = 1.0f;
			result->columns[2].w = 0.0f;
			result->columns[3].x = 0.0f;
			result->columns[3].y = 0.0f;
			result->columns[3].z = 0.0f;
			result->columns[3].w = 1.0f;
			return true;
		case dsRenderSurfaceRotation_180:
			result->columns[0].x = -1.0f;
			result->columns[0].y = 0.0f;
			result->columns[0].z = 0.0f;
			result->columns[0].w = 0.0f;
			result->columns[1].x = 0.0f;
			result->columns[1].y = -1.0f;
			result->columns[1].z = 0.0f;
			result->columns[1].w = 0.0f;
			result->columns[2].x = 0.0f;
			result->columns[2].y = 0.0f;
			result->columns[2].z = 1.0f;
			result->columns[2].w = 0.0f;
			result->columns[3].x = 0.0f;
			result->columns[3].y = 0.0f;
			result->columns[3].z = 0.0f;
			result->columns[3].w = 1.0f;
			return true;
		case dsRenderSurfaceRotation_270:
			result->columns[0].x = 0.0f;
			result->columns[0].y = -1.0f;
			result->columns[0].z = 0.0f;
			result->columns[0].w = 0.0f;
			result->columns[1].x = 1.0f;
			result->columns[1].y = 0.0f;
			result->columns[1].z = 0.0f;
			result->columns[1].w = 0.0f;
			result->columns[2].x = 0.0f;
			result->columns[2].y = 0.0f;
			result->columns[2].z = 1.0f;
			result->columns[2].w = 0.0f;
			result->columns[3].x = 0.0f;
			result->columns[3].y = 0.0f;
			result->columns[3].z = 0.0f;
			result->columns[3].w = 1.0f;
			return true;
		default:
			errno = EINVAL;
			return false;
	}
}

bool dsRenderSurface_rotateViewport(dsAlignedBox3f* result, const dsAlignedBox3f* viewport,
	uint32_t width, uint32_t height, dsRenderSurfaceRotation rotation)
{
	if (!result || !viewport || width == 0 || height == 0)
	{
		errno = EINVAL;
		return false;
	}

	switch (rotation)
	{
		case dsRenderSurfaceRotation_0:
			if (result != viewport)
				*result = *viewport;
			return true;
		case dsRenderSurfaceRotation_90:
		{
			float tempX = viewport->min.x;
			float tempY = viewport->min.y;
			result->min.x = (float)width - viewport->max.y;
			result->min.y = tempX;
			result->min.z = viewport->min.z;
			tempX = viewport->max.x;
			result->max.x = (float)width - tempY;
			result->max.y = tempX;
			result->max.z = viewport->max.z;
			return true;
		}
		case dsRenderSurfaceRotation_180:
		{
			float tempX = viewport->min.x;
			float tempY = viewport->min.y;
			result->min.x = (float)width - viewport->max.x;
			result->min.y = (float)height - viewport->max.y;
			result->min.z = viewport->min.z;
			result->max.x = (float)width - tempX;
			result->max.y = (float)height - tempY;
			result->max.z = viewport->max.z;
			return true;
		}
		case dsRenderSurfaceRotation_270:
		{
			float tempX = viewport->min.x;
			float tempY = viewport->min.y;
			result->min.x = tempY;
			result->min.y = (float)height - viewport->max.x;
			result->min.z = viewport->min.z;
			tempY = viewport->max.y;
			result->max.x = tempY;
			result->max.y = (float)height - tempX;
			result->max.z = viewport->max.z;
			return true;
		}
		default:
			errno = EINVAL;
			return false;
	}
}

bool dsRenderSurface_rotateScissor(dsAlignedBox2f* result, const dsAlignedBox2f* scissor,
	uint32_t width, uint32_t height, dsRenderSurfaceRotation rotation)
{
	if (!result || !scissor || width == 0 || height == 0)
	{
		errno = EINVAL;
		return false;
	}

	switch (rotation)
	{
		case dsRenderSurfaceRotation_0:
			if (result != scissor)
				*result = *scissor;
			return true;
		case dsRenderSurfaceRotation_90:
		{
			float tempX = scissor->min.x;
			float tempY = scissor->min.y;
			result->min.x = (float)width - scissor->max.y;
			result->min.y = tempX;
			tempX = scissor->max.x;
			result->max.x = (float)width - tempY;
			result->max.y = tempX;
			return true;
		}
		case dsRenderSurfaceRotation_180:
		{
			float tempX = scissor->min.x;
			float tempY = scissor->min.y;
			result->min.x = (float)width - scissor->max.x;
			result->min.y = (float)height - scissor->max.y;
			result->max.x = (float)width - tempX;
			result->max.y = (float)height - tempY;
			return true;
		}
		case dsRenderSurfaceRotation_270:
		{
			float tempX = scissor->min.x;
			float tempY = scissor->min.y;
			result->min.x = tempY;
			result->min.y = (float)height - scissor->max.x;
			tempY = scissor->max.y;
			result->max.x = tempY;
			result->max.y = (float)height - tempX;
			return true;
		}
		default:
			errno = EINVAL;
			return false;
	}
}

int dsRenderSurface_isHandleSupported(const dsRenderer* renderer, void* displayHandle,
	void* osHandle, dsRenderSurfaceType type, dsRenderSurfaceColorType colorType)
{
	if (!renderer)
		return -1;

	if (!renderer->renderSurfaceHandleSupportsFormatFunc)
		return true;

	dsRenderSurfaceHint hint;
	switch (colorType)
	{
		case dsRenderSurfaceColorType_SDR:
			DS_VERIFY(dsRenderSurfaceHint_fromFormats(&hint, renderer->sdrSurfaceColorFormat,
				renderer->surfaceDepthStencilFormat, renderer->sdrSurfaceColorSpace, true));
			break;
		case dsRenderSurfaceColorType_HDR:
			if (!dsGfxFormat_isValid(renderer->hdrSurfaceColorFormat))
				return false;
			DS_VERIFY(dsRenderSurfaceHint_fromFormats(&hint, renderer->hdrSurfaceColorFormat,
				renderer->surfaceDepthStencilFormat, renderer->hdrSurfaceColorSpace, true));
			break;
		case dsRenderSurfaceColorType_Preferred:
			DS_VERIFY(dsRenderSurfaceHint_fromFormats(&hint, renderer->preferredSurfaceColorFormat,
				renderer->surfaceDepthStencilFormat, renderer->preferredSurfaceColorSpace, true));
			break;
		default:
			return false;
	}
	return renderer->renderSurfaceHandleSupportsFormatFunc(
		renderer, displayHandle, osHandle, type, &hint, renderer->surfaceSamples);
}

int dsRenderSurface_handleSupportsFormat(const dsRenderer* renderer, void* displayHandle,
	void* osHandle, dsRenderSurfaceType type, const dsRenderSurfaceHint* formatHint,
	uint32_t samples)
{
	if (!renderer || !formatHint)
		return -1;

	if (samples > renderer->maxSurfaceSamples)
		return false;

	if (!renderer->renderSurfaceSupportsFormatFunc)
	{
		return dsRenderSurfaceHint_colorFormat(formatHint, dsGfxFormat_R5G6B5, dsGfxFormat_R8G8B8,
			dsGfxFormat_R8G8B8A8, dsGfxFormat_A2B10G10R10) != dsGfxFormat_Unknown;
	}

	return renderer->renderSurfaceHandleSupportsFormatFunc(
		renderer, displayHandle, osHandle, type, formatHint, samples);
}

dsRenderSurface* dsRenderSurface_create(dsRenderer* renderer, dsAllocator* allocator,
	const char* name, void* displayHandle, void* osHandle, dsRenderSurfaceType type,
	dsRenderSurfaceUsage usage, dsRenderSurfaceColorType colorType, unsigned int widthHint,
	unsigned int heightHint)
{
	DS_PROFILE_FUNC_START();

	if (!renderer || (!allocator && !renderer->allocator) || !renderer->createRenderSurfaceFunc ||
		!renderer->destroyRenderSurfaceFunc || !name)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	switch (colorType)
	{
		case dsRenderSurfaceColorType_SDR:
		case dsRenderSurfaceColorType_Preferred:
			break;
		case dsRenderSurfaceColorType_HDR:
			if (!dsGfxFormat_isValid(renderer->hdrSurfaceColorFormat))
			{
				DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Cannot create an HDR render surface without an "
					"HDR surface format set on the renderer.");
				errno = EPERM;
				DS_PROFILE_FUNC_RETURN(NULL);
			}
			break;
		default:
			errno = EINVAL;
			DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!allocator)
		allocator = renderer->allocator;

	if (!dsThread_equal(dsThread_thisThreadID(), renderer->mainThread))
	{
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Render surfaces may only be created on the main thread.");
		errno = EPERM;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsRenderSurface* renderSurface = renderer->createRenderSurfaceFunc(renderer, allocator, name,
		displayHandle, osHandle, type, usage, colorType, widthHint, heightHint);
	DS_PROFILE_FUNC_RETURN(renderSurface);
}

bool dsRenderSurface_isValid(const dsRenderSurface* renderSurface)
{
	if (!renderSurface || !renderSurface->renderer)
		return false;

	const dsRenderer* renderer = renderSurface->renderer;
	if (!renderer->renderSurfaceSupportsFormatFunc)
		return true;

	dsRenderSurfaceHint formatHint;
	if (!dsRenderSurfaceHint_fromColorType(&formatHint, renderer, renderSurface->colorType))
		return false;

	return renderer->renderSurfaceSupportsFormatFunc(
		renderer, renderSurface, &formatHint, renderer->surfaceSamples);
}

bool dsRenderSurface_supportsColorType(
	const dsRenderSurface* renderSurface, dsRenderSurfaceColorType colorType)
{
	if (!renderSurface || !renderSurface->renderer ||
		!renderSurface->renderer->renderSurfaceSupportsFormatFunc)
	{
		return false;
	}

	const dsRenderer* renderer = renderSurface->renderer;
	dsRenderSurfaceHint formatHint;
	if (!dsRenderSurfaceHint_fromColorType(&formatHint, renderer, colorType))
		return false;

	return renderer->renderSurfaceSupportsFormatFunc(
		renderer, renderSurface, &formatHint, renderer->surfaceSamples);
}

bool dsRenderSurface_supportsFormat(
	const dsRenderSurface* renderSurface, const dsRenderSurfaceHint* formatHint, uint32_t samples)
{
	if (!renderSurface || !renderSurface->renderer ||
		!renderSurface->renderer->renderSurfaceSupportsFormatFunc ||
		samples > renderSurface->renderer->maxSurfaceSamples)
	{
		return false;
	}

	const dsRenderer* renderer = renderSurface->renderer;
	return renderer->renderSurfaceSupportsFormatFunc(renderer, renderSurface, formatHint, samples);
}

bool dsRenderSurface_setColorType(
	dsRenderSurface* renderSurface, dsRenderSurfaceColorType colorType)
{
	if (!renderSurface || !renderSurface->renderer ||
		!renderSurface->renderer->renderSurfaceSupportsFormatFunc)
	{
		errno = EINVAL;
		return false;
	}

	const dsRenderer* renderer = renderSurface->renderer;
	if (!renderer->dynamicRenderSurfaceFormats)
	{
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Current target doesn't support changing the color type on "
			"an existing render surface.");
		errno = EPERM;
		return false;
	}

	if (colorType == dsRenderSurfaceColorType_HDR &&
		!dsGfxFormat_isValid(renderer->hdrSurfaceColorFormat))
	{
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Cannot create an HDR render surface without an "
			"HDR surface format set on the renderer.");
		errno = EPERM;
		return false;
	}

	dsRenderSurfaceHint formatHint;
	if (!dsRenderSurfaceHint_fromColorType(&formatHint, renderer, colorType))
		return false;

	if (!renderer->renderSurfaceSupportsFormatFunc(
			renderer, renderSurface, &formatHint, renderer->surfaceSamples))
	{
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Render surface doesn't support color type.");
		errno = EPERM;
		return false;
	}

	renderSurface->colorType = colorType;
	return true;
}

bool dsRenderSurface_update(
	dsRenderSurface* renderSurface, unsigned int widthHint, unsigned int heightHint)
{
	DS_PROFILE_FUNC_START();

	if (!renderSurface || !renderSurface->renderer ||
		!renderSurface->renderer->updateRenderSurfaceFunc)
	{
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!dsThread_equal(dsThread_thisThreadID(), renderSurface->renderer->mainThread))
	{
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Render surfaces may only be updated on the main thread.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool changed = renderSurface->renderer->updateRenderSurfaceFunc(
		renderSurface->renderer, renderSurface, widthHint, heightHint);
	DS_PROFILE_FUNC_RETURN(changed);
}

bool dsRenderSurface_beginDraw(const dsRenderSurface* renderSurface, dsCommandBuffer* commandBuffer)
{
	beginSurfaceScope(renderSurface);
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !renderSurface || !renderSurface->renderer ||
		!renderSurface->renderer->beginRenderSurfaceFunc ||
		!renderSurface->renderer->endRenderSurfaceFunc)
	{
		DS_PROFILE_FUNC_END();
		endSurfaceScope(renderSurface);
		errno = EINVAL;
		return false;
	}

	if (commandBuffer->usage & dsCommandBufferUsage_Resource)
	{
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Cannot begin drawing to a render surface with a resource command buffer.");
		errno = EPERM;
		return false;
	}

	if (!commandBuffer->frameActive)
	{
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Cannot begin drawing to a render surface outside of a frame.");
		errno = EPERM;
		return false;
	}

	if (commandBuffer->boundSurface)
	{
		DS_PROFILE_FUNC_END();
		endSurfaceScope(renderSurface);
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Cannot begin drawing to a render surface when one is already bound.");
		errno = EPERM;
		return false;
	}

	if (commandBuffer->boundRenderPass)
	{
		DS_PROFILE_FUNC_END();
		endSurfaceScope(renderSurface);
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Cannot begin drawing to a render surface inside of a render pass.");
		errno = EPERM;
		return false;
	}

	if (commandBuffer->usage & dsCommandBufferUsage_Secondary)
	{
		DS_PROFILE_FUNC_END();
		endSurfaceScope(renderSurface);
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Cannot begin drawing to a render surface inside of a secondary command buffer.");
		errno = EPERM;
		return false;
	}

	if (commandBuffer->boundComputeShader)
	{
		DS_PROFILE_FUNC_END();
		endSurfaceScope(renderSurface);
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Cannot begin drawing to a render surface while a compute shader is bound.");
		errno = EPERM;
		return false;
	}

	dsRenderer* renderer = renderSurface->renderer;
	bool begun = renderer->beginRenderSurfaceFunc(renderer, commandBuffer, renderSurface);
	DS_PROFILE_FUNC_END();
	if (begun)
	{
		dsGPUProfileContext_beginSurface(
			renderer->_profileContext, commandBuffer, renderSurface->name);
		commandBuffer->boundSurface = renderSurface;
	}
	else
		endSurfaceScope(renderSurface);
	return begun;
}

bool dsRenderSurface_endDraw(const dsRenderSurface* renderSurface, dsCommandBuffer* commandBuffer)
{
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !renderSurface || !renderSurface->renderer ||
		!renderSurface->renderer->endRenderSurfaceFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer->boundSurface != renderSurface)
	{
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Can only end drawing to the currently bound render surface.");
		errno = EPERM;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer->boundRenderPass)
	{
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Cannot end drawing to a render surface inside of a render pass.");
		errno = EPERM;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer->boundComputeShader)
	{
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Cannot end drawing to a render surface while a compute shader is bound.");
		errno = EPERM;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsRenderer* renderer = renderSurface->renderer;
	bool ended = renderer->endRenderSurfaceFunc(renderer, commandBuffer, renderSurface);
	DS_PROFILE_FUNC_END();
	if (ended)
	{
		dsGPUProfileContext_endSurface(renderer->_profileContext, commandBuffer);
		endSurfaceScope(renderSurface);
		commandBuffer->boundSurface = NULL;
	}
	return ended;
}

bool dsRenderSurface_swapBuffers(dsRenderSurface** renderSurfaces, uint32_t count)
{
	DS_PROFILE_WAIT_START(__FUNCTION__);

	if (count > 0 && !renderSurfaces)
	{
		DS_PROFILE_WAIT_END();
		errno = EINVAL;
		return false;
	}

	if (count == 0)
	{
		DS_PROFILE_WAIT_END();
		return true;
	}

	if (!renderSurfaces[0] || !renderSurfaces[0]->renderer ||
		!renderSurfaces[0]->renderer->swapRenderSurfaceBuffersFunc)
	{
		DS_PROFILE_WAIT_END();
		errno = EINVAL;
		return false;
	}

	for (size_t i = 1; i < count; ++i)
	{
		if (!renderSurfaces[i] || renderSurfaces[i]->renderer != renderSurfaces[0]->renderer)
		{
			DS_PROFILE_WAIT_END();
			errno = EINVAL;
			return false;
		}
	}

	dsRenderer* renderer = renderSurfaces[0]->renderer;
	if (!dsThread_equal(dsThread_thisThreadID(), renderer->mainThread))
	{
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Render surfaces may only be swapped on the main thread.");
		DS_PROFILE_WAIT_END();
		errno = EPERM;
		return false;
	}

	dsGPUProfileContext_beginSwapBuffers(renderer->_profileContext);
	bool swapped = renderer->swapRenderSurfaceBuffersFunc(renderer, renderSurfaces, count);
	dsGPUProfileContext_endSwapBuffers(renderer->_profileContext);
	DS_PROFILE_WAIT_END();
	return swapped;
}

bool dsRenderSurface_destroy(dsRenderSurface* renderSurface)
{
	if (!renderSurface)
		return true;

	DS_PROFILE_FUNC_START();

	if (!renderSurface->renderer || !renderSurface->renderer->destroyRenderSurfaceFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsRenderer* renderer = renderSurface->renderer;
	if (!dsThread_equal(dsThread_thisThreadID(), renderer->mainThread))
	{
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Render surfaces may only be destroyed on the main thread.");
		errno = EPERM;
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool destroyed = renderer->destroyRenderSurfaceFunc(renderer, renderSurface);
	DS_PROFILE_FUNC_RETURN(destroyed);
}
