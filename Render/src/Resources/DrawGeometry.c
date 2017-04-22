/*
 * Copyright 2016 Aaron Barany
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

#include <DeepSea/Render/Resources/DrawGeometry.h>

#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Bits.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/ResourceManager.h>
#include <DeepSea/Render/Resources/VertexFormat.h>
#include <DeepSea/Render/Types.h>

extern const char* dsResourceManager_noContextError;

dsDrawGeometry* dsDrawGeometry_create(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsVertexBuffer* vertexBuffers[DS_MAX_GEOMETRY_VERTEX_BUFFERS],
	dsIndexBuffer* indexBuffer)
{
	DS_PROFILE_FUNC_START();

	if (!resourceManager || (!allocator && !resourceManager->allocator) ||
		!resourceManager->createGeometryFunc || !resourceManager->destroyGeometryFunc ||
		!vertexBuffers)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!allocator)
		allocator = resourceManager->allocator;

	bool hasVertexBuffer = false;
	uint32_t vertexCount = 0;
	for (unsigned int i = 0; i < DS_MAX_GEOMETRY_VERTEX_BUFFERS; ++i)
	{
		if (!vertexBuffers[i])
			continue;

		if (hasVertexBuffer)
		{
			if (vertexBuffers[i]->count != vertexCount)
			{
				errno = EPERM;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG,
					"Vertex buffers must have the same number of vertices.");
				DS_PROFILE_FUNC_RETURN(NULL);
			}
		}
		else
		{
			hasVertexBuffer = true;
			vertexCount = vertexBuffers[i]->count;
		}

		if (!vertexBuffers[i]->buffer)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Vertex buffer doesn't contain a graphics buffer.");
			DS_PROFILE_FUNC_RETURN(NULL);
		}

		if (!(vertexBuffers[i]->buffer->usage & dsGfxBufferUsage_Vertex))
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Graphics buffer not created as a vertex buffer.");
			DS_PROFILE_FUNC_RETURN(NULL);
		}

		if (!dsVertexFormat_isValid(resourceManager, &vertexBuffers[i]->format))
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Invalid vertex format.");
			DS_PROFILE_FUNC_RETURN(NULL);
		}

		if (vertexBuffers[i]->format.size == 0)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Offsets and size not calculated for vertex format.");
			DS_PROFILE_FUNC_RETURN(NULL);
		}

		if (vertexBuffers[i]->offset + vertexBuffers[i]->count*vertexBuffers[i]->format.size >
			vertexBuffers[i]->buffer->size)
		{
			errno = EINDEX;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG,
				"Vertex buffer range is outside of graphics buffer range.");
			DS_PROFILE_FUNC_RETURN(NULL);
		}
	}

	if (!hasVertexBuffer)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Creating a draw geometry without a vertex buffer.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (indexBuffer)
	{
		if (!indexBuffer->buffer)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Index buffer doesn't contain a graphics buffer.");
			DS_PROFILE_FUNC_RETURN(NULL);
		}

		if (!(indexBuffer->buffer->usage & dsGfxBufferUsage_Index))
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Graphics buffer not created as an index buffer.");
			DS_PROFILE_FUNC_RETURN(NULL);
		}

		uint32_t indexBits = indexBuffer->indexBits;
		if (indexBits > resourceManager->maxIndexBits || (indexBits != 16 && indexBits != 32))
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Invalid number of index bits.");
			DS_PROFILE_FUNC_RETURN(NULL);
		}

		if (indexBuffer->offset + indexBuffer->count*indexBits/8 > indexBuffer->buffer->size)
		{
			errno = EINDEX;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG,
				"Index buffer range is outside of graphics buffer range.");
			DS_PROFILE_FUNC_RETURN(NULL);
		}
	}

	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, dsResourceManager_noContextError);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsDrawGeometry* geometry = resourceManager->createGeometryFunc(resourceManager, allocator,
		vertexBuffers, indexBuffer);
	if (geometry)
		DS_ATOMIC_FETCH_ADD32(&resourceManager->geometryCount, 1);
	DS_PROFILE_FUNC_RETURN(geometry);
}

uint32_t dsDrawGeometry_getVertexCount(const dsDrawGeometry* geometry)
{
	if (!geometry)
		return 0;

	for (unsigned int i = 0; i < DS_MAX_GEOMETRY_VERTEX_BUFFERS; ++i)
	{
		if (geometry->vertexBuffers[i].buffer)
			return geometry->vertexBuffers[i].count;
	}

	return 0;
}

uint32_t dsDrawGeometry_getIndexCount(const dsDrawGeometry* geometry)
{
	if (!geometry || !geometry->indexBuffer.buffer)
		return 0;

	return geometry->indexBuffer.count;
}

bool dsDrawGeometry_destroy(dsDrawGeometry* geometry)
{
	DS_PROFILE_FUNC_START();

	if (!geometry || !geometry->resourceManager || !geometry->resourceManager->destroyGeometryFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = geometry->resourceManager;
	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, dsResourceManager_noContextError);
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = resourceManager->destroyGeometryFunc(resourceManager, geometry);
	if (success)
		DS_ATOMIC_FETCH_ADD32(&resourceManager->geometryCount, -1);
	DS_PROFILE_FUNC_RETURN(success);
}
