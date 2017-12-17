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

#include <DeepSea/Render/Renderer.h>

#include <DeepSea/Core/Thread/Thread.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Geometry/Frustum3.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Render/Resources/DrawGeometry.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/ResourceManager.h>
#include <string.h>

static bool isDepthStencil(dsGfxFormat format)
{
	return format >= dsGfxFormat_D16 && format <= dsGfxFormat_D32S8_Float;
}

static bool getBlitSurfaceInfo(dsGfxFormat* outFormat, dsTextureDim* outDim, uint32_t* outWidth,
	uint32_t* outHeight, uint32_t* outLayers, uint32_t* outMipLevels, const dsRenderer* renderer,
	dsGfxSurfaceType surfaceType, void* surface, bool read)
{
	switch (surfaceType)
	{
		case dsGfxSurfaceType_ColorRenderSurfaceLeft:
		case dsGfxSurfaceType_ColorRenderSurfaceRight:
			if (!renderer->stereoscopic)
			{
				errno = EPERM;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG,
					"Attempting to use a stereoscopic render surface for a blit when  not using "
					"stereoscopic rendering.");
				return false;
			}
			// fall through
		case dsGfxSurfaceType_ColorRenderSurface:
		{
			dsRenderSurface* realSurface = (dsRenderSurface*)surface;
			*outFormat = renderer->surfaceColorFormat;
			*outDim = dsTextureDim_2D;
			*outWidth = realSurface->width;
			*outHeight = realSurface->height;
			*outLayers = 1;
			*outMipLevels = 1;
			break;
		}
		case dsGfxSurfaceType_DepthRenderSurfaceLeft:
		case dsGfxSurfaceType_DepthRenderSurfaceRight:
			if (!renderer->stereoscopic)
			{
				errno = EPERM;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG,
					"Attempting to use a stereoscopic render surface for a blit when  not using "
					"stereoscopic rendering.");
				return false;
			}
			// fall through
		case dsGfxSurfaceType_DepthRenderSurface:
		{
			dsRenderSurface* realSurface = (dsRenderSurface*)surface;
			*outFormat = renderer->surfaceDepthStencilFormat;
			*outDim = dsTextureDim_2D;
			*outWidth = realSurface->width;
			*outHeight = realSurface->height;
			*outLayers = 1;
			*outMipLevels = 1;

			if (renderer->surfaceSamples > 1)
			{
				errno = EPERM;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Cannot blit mipmapped surfaces.");
				DS_PROFILE_FUNC_RETURN(false);
			}
			break;
		}
		case dsGfxSurfaceType_Texture:
		{
			dsTexture* realSurface = (dsTexture*)surface;
			*outFormat = realSurface->format;
			*outDim = realSurface->dimension;
			*outWidth = realSurface->width;
			*outHeight = realSurface->height;
			if (realSurface->resolve)
			{
				*outLayers = 1;
				*outMipLevels = realSurface->mipLevels;
			}
			else
			{
				*outLayers = dsMax(1U, realSurface->depth);
				*outMipLevels = realSurface->mipLevels;
			}

			if (read && !(realSurface->usage & dsTextureUsage_CopyFrom))
			{
				errno = EPERM;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG,
					"Attempting to copy data from a texture without the copy from usage flag set.");
				DS_PROFILE_FUNC_RETURN(false);
			}
			else if (!read && !(realSurface->usage & dsTextureUsage_CopyTo))
			{
				errno = EPERM;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG,
					"Attempting to copy data to a texture without the copy to usage flag set.");
				DS_PROFILE_FUNC_RETURN(false);
			}

			if (!realSurface->resolve && realSurface->samples > 1)
			{
				errno = EPERM;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Cannot blit mipmapped surfaces.");
				DS_PROFILE_FUNC_RETURN(false);
			}

			break;
		}
		case dsGfxSurfaceType_Renderbuffer:
		{
			dsRenderbuffer* realSurface = (dsRenderbuffer*)surface;
			*outFormat = realSurface->format;
			*outDim = dsTextureDim_2D;
			*outWidth = realSurface->width;
			*outHeight = realSurface->height;
			*outLayers = 1;
			*outMipLevels = 1;

			if (realSurface->samples > 1)
			{
				errno = EPERM;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Cannot blit mipmapped surfaces.");
				DS_PROFILE_FUNC_RETURN(false);
			}
			break;
		}
		default:
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Unknown surface type.");
			return false;
	}

	return true;
}

bool dsRenderer_makeOrtho(dsMatrix44f* result, const dsRenderer* renderer, float left, float right,
	float bottom, float top, float near, float far)
{
	if (!result || !renderer ||  left == right || bottom == top || near == far)
	{
		errno = EINVAL;
		return false;
	}

	dsMatrix44f_makeOrtho(result, left, right, bottom, top, near, far, renderer->clipHalfDepth,
		renderer->clipInvertY);
	return true;
}

bool dsRenderer_makeFrustum(dsMatrix44f* result, const dsRenderer* renderer, float left,
	float right, float bottom, float top, float near, float far)
{
	if (!result || !renderer ||  left == right || bottom == top || near == far)
	{
		errno = EINVAL;
		return false;
	}

	dsMatrix44f_makeFrustum(result, left, right, bottom, top, near, far, renderer->clipHalfDepth,
		renderer->clipInvertY);
	return true;
}

bool dsRenderer_makePerspective(dsMatrix44f* result, const dsRenderer* renderer, float fovy,
	float aspect, float near, float far)
{
	if (!result || !renderer ||  fovy == 0 || aspect == 0|| near == far)
	{
		errno = EINVAL;
		return false;
	}

	dsMatrix44f_makePerspective(result, fovy, aspect, near, far, renderer->clipHalfDepth,
		renderer->clipInvertY);
	return true;
}

bool dsRenderer_frustumFromMatrix(dsFrustum3f* result, const dsRenderer* renderer,
	const dsMatrix44f* matrix)
{
	if (!result || !renderer || !matrix)
	{
		errno = EINVAL;
		return false;
	}

	dsFrustum3_fromMatrix(*result, *matrix, renderer->clipHalfDepth, renderer->clipInvertY);
	return true;
}

bool dsRenderer_beginFrame(dsRenderer* renderer)
{
	if (!renderer || !renderer->beginFrameFunc || !renderer->endFrameFunc)
	{
		errno = EINVAL;
		return false;
	}

	if (!dsThread_equal(dsThread_thisThreadId(), renderer->mainThread))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Frames may only be begun on the main thread.");
		return false;
	}

	dsProfile_startFrame();
	return renderer->beginFrameFunc(renderer);
}

bool dsRenderer_endFrame(dsRenderer* renderer)
{
	if (!renderer || !renderer->beginFrameFunc || !renderer->endFrameFunc)
	{
		errno = EINVAL;
		return false;
	}

	if (!dsThread_equal(dsThread_thisThreadId(), renderer->mainThread))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Frames may only be ended on the main thread.");
		return false;
	}

	bool success = renderer->endFrameFunc(renderer);
	dsResourceManager_reportStatistics(renderer->resourceManager);
	dsProfile_endFrame();
	return success;
}

bool dsRenderer_setSurfaceSamples(dsRenderer* renderer, uint32_t samples)
{
	DS_PROFILE_FUNC_START();

	if (!renderer || !renderer->setSurfaceSamplesFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (samples > renderer->maxSurfaceSamples)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Surface samples is above the maximume.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!dsThread_equal(dsThread_thisThreadId(), renderer->mainThread))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Surface samples may only be set on the main thread.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = renderer->setSurfaceSamplesFunc(renderer, samples);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsRenderer_setVsync(dsRenderer* renderer, bool vsync)
{
	DS_PROFILE_FUNC_START();

	if (!renderer || !renderer->setVsyncFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!dsThread_equal(dsThread_thisThreadId(), renderer->mainThread))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Vsync may only be set on the main thread.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = renderer->setVsyncFunc(renderer, vsync);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsRenderer_setDefaultAnisotropy(dsRenderer* renderer, float anisotropy)
{
	DS_PROFILE_FUNC_START();

	if (!renderer || !renderer->setDefaultAnisotropyFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (anisotropy > renderer->maxAnisotropy)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Anisotropy is above the maximum.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!dsThread_equal(dsThread_thisThreadId(), renderer->mainThread))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Default anisotropy may only be set on the main thread.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = renderer->setDefaultAnisotropyFunc(renderer, anisotropy);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsRenderer_clearColorSurface(dsCommandBuffer* commandBuffer, dsRenderer* renderer,
	const dsFramebufferSurface* surface, const dsSurfaceColorValue* colorValue)
{
	DS_PROFILE_FUNC_START();

	if (!renderer || !renderer->clearColorSurfaceFunc || !commandBuffer || !surface ||
		!surface->surface || !colorValue)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool valid;
	switch (surface->surfaceType)
	{
		case dsGfxSurfaceType_ColorRenderSurface:
		case dsGfxSurfaceType_ColorRenderSurfaceLeft:
		case dsGfxSurfaceType_ColorRenderSurfaceRight:
			valid = true;
			break;
		case dsGfxSurfaceType_DepthRenderSurface:
		case dsGfxSurfaceType_DepthRenderSurfaceLeft:
		case dsGfxSurfaceType_DepthRenderSurfaceRight:
			valid = false;
			break;
		case dsGfxSurfaceType_Texture:
		{
			dsOffscreen* offscreen = (dsOffscreen*)surface->surface;
			if (!offscreen->offscreen)
			{
				errno = EPERM;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to clear a non-offscreen texture.");
				DS_PROFILE_FUNC_RETURN(false);
			}

			uint32_t surfaceLayers = dsMax(1U, offscreen->depth);
			if (offscreen->dimension == dsTextureDim_Cube)
				surfaceLayers *= 6;

			if (surface->mipLevel >= offscreen->mipLevels)
			{
				errno = EINDEX;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Mip level out of range for offscreen.");
				DS_PROFILE_FUNC_RETURN(false);
			}

			if (surface->layer >= surfaceLayers)
			{
				errno = EINDEX;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG,
					"Texture layer out of range for offscreen.");
				DS_PROFILE_FUNC_RETURN(false);
			}
			valid = !isDepthStencil(offscreen->format);
			break;
		}
		case dsGfxSurfaceType_Renderbuffer:
			valid = !isDepthStencil(((dsRenderbuffer*)surface->surface)->format);
			break;
		default:
			DS_ASSERT(false);
			valid = false;
			break;
	}

	if (!valid)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Cannot clear a depth-stencil surface as a color surface.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = renderer->clearColorSurfaceFunc(renderer, commandBuffer, surface, colorValue);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsRenderer_clearDepthStencilSurface(dsCommandBuffer* commandBuffer, dsRenderer* renderer,
	const dsFramebufferSurface* surface, dsClearDepthStencil surfaceParts,
	const dsDepthStencilValue* depthStencilValue)
{
	DS_PROFILE_FUNC_START();

	if (!renderer || !renderer->clearDepthStencilSurfaceFunc || !commandBuffer || !surface ||
		!surface->surface || !depthStencilValue)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool valid;
	switch (surface->surfaceType)
	{
		case dsGfxSurfaceType_ColorRenderSurface:
			valid = false;
			break;
		case dsGfxSurfaceType_DepthRenderSurface:
			valid = true;
			break;
		case dsGfxSurfaceType_Texture:
		{
			dsOffscreen* offscreen = (dsOffscreen*)surface->surface;
			if (!offscreen->offscreen)
			{
				errno = EPERM;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to clear a non-offscreen texture.");
				DS_PROFILE_FUNC_RETURN(false);
			}

			uint32_t surfaceLayers = dsMax(1U, offscreen->depth);
			if (offscreen->dimension == dsTextureDim_Cube)
				surfaceLayers *= 6;

			if (surface->mipLevel >= offscreen->mipLevels)
			{
				errno = EINDEX;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Mip level out of range for offscreen.");
				DS_PROFILE_FUNC_RETURN(false);
			}

			if (surface->layer >= surfaceLayers)
			{
				errno = EINDEX;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG,
					"Texture layer out of range for offscreen.");
				DS_PROFILE_FUNC_RETURN(false);
			}
			valid = isDepthStencil(offscreen->format);
			break;
		}
		case dsGfxSurfaceType_Renderbuffer:
			valid = isDepthStencil(((dsRenderbuffer*)surface->surface)->format);
			break;
		default:
			DS_ASSERT(false);
			valid = false;
			break;
	}

	if (!valid)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Cannot clear a color surface as a depth-stencil surface.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = renderer->clearDepthStencilSurfaceFunc(renderer, commandBuffer, surface,
		surfaceParts, depthStencilValue);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsRenderer_draw(dsCommandBuffer* commandBuffer, dsRenderer* renderer,
	const dsDrawGeometry* geometry, const dsDrawRange* drawRange)
{
	DS_PROFILE_FUNC_START();

	if (!renderer || !renderer->drawFunc || !commandBuffer || !geometry || !drawRange)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	uint32_t vertexCount = dsDrawGeometry_getVertexCount(geometry);
	if (!DS_IS_BUFFER_RANGE_VALID(drawRange->firstVertex, drawRange->vertexCount, vertexCount))
	{
		errno = EINDEX;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Draw range is out of range of geometry vertices.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!renderer->supportsInstancedDrawing && (drawRange->firstInstance != 0 ||
		drawRange->instanceCount != 1))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Current target doesn't support instanced drawing. Must "
			"draw a single instance of index 0.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!renderer->supportsStartInstance && drawRange->firstInstance != 0 )
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Current target doesn't support setting the start instance.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = renderer->drawFunc(renderer, commandBuffer, geometry, drawRange);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsRenderer_drawIndexed(dsCommandBuffer* commandBuffer, dsRenderer* renderer,
	const dsDrawGeometry* geometry, const dsDrawIndexedRange* drawRange)
{
	DS_PROFILE_FUNC_START();

	if (!renderer || !renderer->drawFunc || !commandBuffer || !geometry || !drawRange)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	uint32_t indexCount = dsDrawGeometry_getIndexCount(geometry);
	if (indexCount == 0)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Geometry must contain indices for indexed drawing.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!DS_IS_BUFFER_RANGE_VALID(drawRange->firstIndex, drawRange->indexCount, indexCount))
	{
		errno = EINDEX;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Draw range is out of range of geometry indices.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!renderer->supportsInstancedDrawing && (drawRange->firstInstance != 0 ||
		drawRange->instanceCount != 1))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Current target doesn't support instanced drawing. Must "
			"draw a single instance of index 0.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!renderer->supportsStartInstance && drawRange->firstInstance != 0 )
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Current target doesn't support setting the start instance.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = renderer->drawIndexedFunc(renderer, commandBuffer, geometry, drawRange);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsRenderer_drawIndirect(dsCommandBuffer* commandBuffer, dsRenderer* renderer,
	const dsDrawGeometry* geometry, const dsGfxBuffer* indirectBuffer, size_t offset,
	uint32_t count, uint32_t stride)
{
	DS_PROFILE_FUNC_START();

	if (!renderer || !renderer->drawIndirectFunc || !commandBuffer || !geometry || !indirectBuffer)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!(indirectBuffer->usage & dsGfxBufferUsage_IndirectDraw))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Graphics buffer not created as an indirect buffer.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (stride < sizeof(dsDrawRange))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Indirect buffer must contain members of type dsDrawRange.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (offset % sizeof(uint32_t) != 0)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Indirect buffer must be aligned with uint32_t.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!DS_IS_BUFFER_RANGE_VALID(offset, count*stride, indirectBuffer->size))
	{
		errno = EINDEX;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Indirect draws outside of indirect buffer range.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = renderer->drawIndirectFunc(renderer, commandBuffer, geometry, indirectBuffer,
		offset, count, stride);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsRenderer_drawIndexedIndirect(dsCommandBuffer* commandBuffer, dsRenderer* renderer,
	const dsDrawGeometry* geometry, const dsGfxBuffer* indirectBuffer, size_t offset,
	uint32_t count, uint32_t stride)
{
	DS_PROFILE_FUNC_START();

	if (!renderer || !renderer->drawIndexedIndirectFunc || !commandBuffer || !geometry ||
		!indirectBuffer)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!(indirectBuffer->usage & dsGfxBufferUsage_IndirectDraw))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Graphics buffer not created as an indirect buffer.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (stride < sizeof(dsDrawIndexedRange))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Indirect buffer must contain members of type dsDrawIndexedRange.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (offset % sizeof(uint32_t) != 0)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Indirect buffer must be aligned with uint32_t.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!DS_IS_BUFFER_RANGE_VALID(offset, count*stride, indirectBuffer->size))
	{
		errno = EINDEX;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Indirect draws outside of indirect buffer range.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (dsDrawGeometry_getIndexCount(geometry) == 0)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Geometry must contain indices for indexed drawing.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = renderer->drawIndexedIndirectFunc(renderer, commandBuffer, geometry,
		indirectBuffer, offset, count, stride);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsRenderer_dispatchCompute(dsCommandBuffer* commandBuffer, dsRenderer* renderer, uint32_t x,
	uint32_t y, uint32_t z)
{
	DS_PROFILE_FUNC_START();

	if (!renderer || !commandBuffer)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!renderer->dispatchComputeFunc || !renderer->hasComputeShaders)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Current target doesn't support compute shaders.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = renderer->dispatchComputeFunc(renderer, commandBuffer, x, y, z);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsRenderer_dispatchComputeIndirect(dsCommandBuffer* commandBuffer, dsRenderer* renderer,
	const dsGfxBuffer* indirectBuffer, size_t offset)
{
	DS_PROFILE_FUNC_START();

	if (!renderer || !commandBuffer || !indirectBuffer)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!renderer->dispatchComputeIndirectFunc || !renderer->hasComputeShaders)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Current target doesn't support compute shaders.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!(indirectBuffer->usage & dsGfxBufferUsage_IndirectDispatch))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Graphics buffer not created as an indirect buffer.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (offset % sizeof(uint32_t) != 0)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Indirect buffer must be aligned with uint32_t.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!DS_IS_BUFFER_RANGE_VALID(offset, 3*sizeof(uint32_t), indirectBuffer->size))
	{
		errno = EINDEX;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Indirect dispatch outside of indirect buffer range.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = renderer->dispatchComputeIndirectFunc(renderer, commandBuffer, indirectBuffer,
		offset);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsRenderer_blitSurface(dsCommandBuffer* commandBuffer, dsRenderer* renderer,
	dsGfxSurfaceType srcSurfaceType, void* srcSurface, dsGfxSurfaceType dstSurfaceType,
	void* dstSurface, const dsSurfaceBlitRegion* regions, size_t regionCount, dsBlitFilter filter)
{
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !renderer || !srcSurface || !dstSurface || !renderer->blitSurfaceFunc ||
		(!regions && regionCount > 0))
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsGfxFormat srcFormat;
	dsTextureDim srcDim;
	uint32_t srcWidth, srcHeight, srcLayers, srcMipLevels;
	if (!getBlitSurfaceInfo(&srcFormat, &srcDim, &srcWidth, &srcHeight, &srcLayers, &srcMipLevels,
		renderer, srcSurfaceType, srcSurface, true))
	{
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsGfxFormat dstFormat;
	dsTextureDim dstDim;
	uint32_t dstWidth, dstHeight, dstLayers, dstMipLevels;
	if (!getBlitSurfaceInfo(&dstFormat, &dstDim, &dstWidth, &dstHeight, &dstLayers, &dstMipLevels,
		renderer, dstSurfaceType, dstSurface, false))
	{
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = renderer->resourceManager;
	if (!dsGfxFormat_surfaceBlitSupported(resourceManager, srcFormat, dstFormat, filter))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Textures cannot be blit between each other on the current target.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	unsigned int srcBlockX, srcBlockY;
	DS_VERIFY(dsGfxFormat_blockDimensions(&srcBlockX, &srcBlockY, srcFormat));
	unsigned int dstBlockX, dstBlockY;
	DS_VERIFY(dsGfxFormat_blockDimensions(&dstBlockX, &dstBlockY, dstFormat));

	for (size_t i = 0; i < regionCount; ++i)
	{
		if (regions[i].srcPosition.x % srcBlockX != 0 || regions[i].srcPosition.y % srcBlockY != 0)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG,
				"Texture data width and height must be a multiple of the block size.");
			DS_PROFILE_FUNC_RETURN(false);
		}

		const dsTexturePosition* srcPosition = &regions[i].srcPosition;
		if (srcPosition->mipLevel >= srcMipLevels)
		{
			errno = EINDEX;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to copy texture data out of range.");
			DS_PROFILE_FUNC_RETURN(false);
		}

		uint32_t srcMipWidth = dsMax(1U, srcWidth >> srcPosition->mipLevel);
		uint32_t srcMipHeight = dsMax(1U, srcHeight >> srcPosition->mipLevel);
		uint32_t srcMipLayers = dsMax(1U, srcLayers);
		uint32_t srcLayerOffset = srcPosition->depth;
		if (srcDim == dsTextureDim_3D)
			srcMipLayers = dsMax(1U, srcMipLayers >> srcPosition->mipLevel);
		else if (srcDim == dsTextureDim_Cube)
		{
			srcMipLayers *= 6;
			srcLayerOffset = srcLayerOffset*6 + srcPosition->face;
		}
		uint32_t srcEndX = srcPosition->x + regions[i].srcWidth;
		uint32_t srcEndY = srcPosition->y + regions[i].srcHeight;
		uint32_t srcEndLayer = srcLayerOffset + regions[i].layers;
		if (srcEndX > srcMipWidth || srcEndY > srcMipHeight || srcEndLayer > srcMipLayers)
		{
			errno = EINDEX;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to copy texture data out of range.");
			DS_PROFILE_FUNC_RETURN(false);
		}

		if ((srcEndX % srcBlockX != 0 && srcEndX != srcMipWidth) ||
			(srcEndY % srcBlockY != 0 && srcEndY != srcMipHeight))
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG,
				"Texture data width and height must be a multiple of the block size or reach the "
				"edge of the image.");
			DS_PROFILE_FUNC_RETURN(false);
		}

		if (regions[i].dstPosition.x % dstBlockX != 0 || regions[i].dstPosition.y % dstBlockY != 0)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG,
				"Texture data position must be a multiple of the block size.");
			DS_PROFILE_FUNC_RETURN(false);
		}

		const dsTexturePosition* dstPosition = &regions[i].dstPosition;
		if (dstPosition->mipLevel >= dstMipLevels)
		{
			errno = EINDEX;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to copy texture data out of range.");
			DS_PROFILE_FUNC_RETURN(false);
		}

		uint32_t dstMipWidth = dsMax(1U, dstWidth >> dstPosition->mipLevel);
		uint32_t dstMipHeight = dsMax(1U, dstHeight >> dstPosition->mipLevel);
		uint32_t dstMipLayers = dsMax(1U, dstLayers);
		uint32_t dstLayerOffset = dstPosition->depth;
		if (dstDim == dsTextureDim_3D)
			dstMipLayers = dsMax(1U, dstMipLayers >> dstPosition->mipLevel);
		else if (dstDim == dsTextureDim_Cube)
		{
			dstMipLayers *= 6;
			dstLayerOffset = dstLayerOffset*6 + dstPosition->face;
		}
		uint32_t dstEndX = regions[i].dstPosition.x + regions[i].dstWidth;
		uint32_t dstEndY = regions[i].dstPosition.y + regions[i].dstHeight;
		uint32_t dstEndLayer = dstLayerOffset + regions[i].layers;
		if (dstEndX > dstMipWidth || dstEndY > dstMipHeight || dstEndLayer > dstMipLayers)
		{
			errno = EINDEX;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to copy texture data out of range.");
			DS_PROFILE_FUNC_RETURN(false);
		}

		if ((dstEndX % dstBlockX != 0 && dstEndX != dstMipWidth) ||
			(dstEndY % dstBlockY != 0 && dstEndY != dstMipHeight))
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG,
				"Texture data width and height must be a multiple of the block size or reach the "
				"edge of the image.");
			DS_PROFILE_FUNC_RETURN(false);
		}
	}

	bool success = renderer->blitSurfaceFunc(renderer, commandBuffer, srcSurfaceType, srcSurface,
		dstSurfaceType, dstSurface, regions, regionCount, filter);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsRenderer_waitUntilIdle(dsRenderer* renderer)
{
	if (!renderer || !renderer->waitUntilIdleFunc)
	{
		errno = EINVAL;
		return false;
	}

	if (!dsThread_equal(dsThread_thisThreadId(), renderer->mainThread))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Waiting for idle must be done on the main thread.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	return renderer->waitUntilIdleFunc(renderer);
}

bool dsRenderer_restoreGlobalState(dsRenderer* renderer)
{
	if (!renderer)
	{
		errno = EINVAL;
		return false;
	}

	// It's valid if there's no global state to restore.
	if (!renderer->restoreGlobalStateFunc)
		return true;

	if (!dsThread_equal(dsThread_thisThreadId(), renderer->mainThread))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Restoring the global state must be done on the main thread.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	return renderer->restoreGlobalStateFunc(renderer);
}

bool dsRenderer_initialize(dsRenderer* renderer)
{
	if (!renderer)
	{
		errno = EINVAL;
		return false;
	}

	memset(renderer, 0, sizeof(dsRenderer));
	renderer->mainThread = dsThread_thisThreadId();
	return true;
}

void dsRenderer_shutdown(dsRenderer* renderer)
{
	DS_UNUSED(renderer);
}
