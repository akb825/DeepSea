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

#include "Resources/GLGfxQueryPool.h"

#include "AnyGL/AnyGL.h"
#include "AnyGL/gl.h"
#include "GLCommandBuffer.h"
#include "GLHelpers.h"
#include "GLResource.h"
#include "GLTypes.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <string.h>

dsGfxQueryPool* dsGLGfxQueryPool_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsGfxQueryType type, uint32_t count)
{
	DS_ASSERT(allocator);

	dsGLRenderer* glRenderer = (dsGLRenderer*)resourceManager->renderer;
	dsGLGfxQueryPool* queries = (dsGLGfxQueryPool*)dsAllocator_alloc(allocator,
		sizeof(dsGLGfxQueryPool) + count*sizeof(GLuint));
	if (!queries)
		return NULL;

	dsGfxQueryPool* baseQueries = (dsGfxQueryPool*)queries;
	baseQueries->resourceManager = resourceManager;
	baseQueries->allocator = dsAllocator_keepPointer(allocator);
	baseQueries->type = type;
	baseQueries->count = count;

	dsGLResource_initialize(&queries->resource);
	queries->queryContext = glRenderer->contextCount;

	//glGenQueries(count, queries->queryIds);
	// Garbage drivers do garbage things, such return the same IDs for glGenQueries() multiple
	// times. Work around this garbage by delaying the creation of the queries.
	memset(queries->queryIds, 0, count*sizeof(GLuint));
	return baseQueries;
}

bool dsGLGfxQueryPool_reset(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsGfxQueryPool* queries, uint32_t first, uint32_t count)
{
	DS_UNUSED(resourceManager);
	DS_UNUSED(commandBuffer);
	DS_UNUSED(queries);
	DS_UNUSED(first);
	DS_UNUSED(count);

	return true;
}

bool dsGLGfxQueryPool_beginQuery(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsGfxQueryPool* queries, uint32_t query)
{
	DS_UNUSED(resourceManager);
	return dsGLCommandBuffer_beginQuery(commandBuffer, queries, query);
}

bool dsGLGfxQueryPool_endQuery(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsGfxQueryPool* queries, uint32_t query)
{
	DS_UNUSED(resourceManager);
	return dsGLCommandBuffer_endQuery(commandBuffer, queries, query);
}

bool dsGLGfxQueryPool_queryTimestamp(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, dsGfxQueryPool* queries, uint32_t query)
{
	DS_UNUSED(resourceManager);
	return dsGLCommandBuffer_queryTimestamp(commandBuffer, queries, query);
}

bool dsGLGfxQueryPool_getValues(dsResourceManager* resourceManager, dsGfxQueryPool* queries,
	uint32_t first, uint32_t count, void* data, size_t dataSize, size_t stride, size_t elementSize,
	bool checkAvailability)
{
	DS_UNUSED(dataSize);

	DS_ASSERT(elementSize == sizeof(uint32_t) || elementSize == sizeof(uint64_t));
	dsGLGfxQueryPool* glQueries = (dsGLGfxQueryPool*)queries;

	// Context re-created.
	dsGLRenderer* glRenderer = (dsGLRenderer*)resourceManager->renderer;
	if (glQueries->queryContext != glRenderer->contextCount)
	{
		memset(glQueries->queryIds, 0, sizeof(GLint)*queries->count);
		glQueries->queryContext = glRenderer->contextCount;
	}

	uint8_t* dataBytes = (uint8_t*)data;
	for (uint32_t i = 0; i < count; ++i, dataBytes += stride)
	{
		// Query needed to be re-allocated. (context destroyed)
		if (!glQueries->queryIds[first + i])
		{
			if (checkAvailability)
			{
				uint8_t* availableBytes = dataBytes + elementSize;
				if (elementSize == sizeof(uint64_t))
					*(uint64_t*)availableBytes = false;
				else
					*(uint32_t*)availableBytes = false;
			}

			if (elementSize == sizeof(uint64_t))
				*(uint64_t*)dataBytes = 0;
			else
				*(uint32_t*)dataBytes = 0;
			continue;
		}

		GLuint ready = true;
		if (checkAvailability)
		{
			uint8_t* availableBytes = dataBytes + elementSize;
			if (elementSize == sizeof(uint64_t))
			{
				glGetQueryObjectui64v(glQueries->queryIds[first + i], GL_QUERY_RESULT_AVAILABLE,
					(uint64_t*)availableBytes);
				ready = *(uint64_t*)availableBytes != 0;
			}
			else
			{
				glGetQueryObjectuiv(glQueries->queryIds[first + i], GL_QUERY_RESULT_AVAILABLE,
					(uint32_t*)availableBytes);
				ready = *(uint32_t*)availableBytes != 0;
			}
		}

		if (!ready)
			continue;

		if (elementSize == sizeof(uint64_t))
		{
			glGetQueryObjectui64v(glQueries->queryIds[first + i], GL_QUERY_RESULT,
				(uint64_t*)dataBytes);
		}
		else
		{
			glGetQueryObjectuiv(glQueries->queryIds[first + i], GL_QUERY_RESULT,
				(uint32_t*)dataBytes);
		}
	}

	return true;
}

bool dsGLGfxQueryPool_copyValues(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, dsGfxQueryPool* queries, uint32_t first, uint32_t count,
	dsGfxBuffer* buffer, size_t offset, size_t stride, size_t elementSize, bool checkAvailability)
{
	DS_UNUSED(resourceManager);
	return dsGLCommandBuffer_copyQueryValues(commandBuffer, queries, first, count, buffer, offset,
		stride, elementSize, checkAvailability);
}

static bool destroyImpl(dsGfxQueryPool* queries)
{
	dsGLGfxQueryPool* glQueries = (dsGLGfxQueryPool*)queries;
	glDeleteQueries(queries->count, glQueries->queryIds);
	if (queries->allocator)
		return dsAllocator_free(queries->allocator, queries);

	return true;
}

bool dsGLGfxQueryPool_destroy(dsResourceManager* resourceManager, dsGfxQueryPool* queries)
{
	DS_UNUSED(resourceManager);
	DS_ASSERT(queries);

	dsGLGfxQueryPool* glQueries = (dsGLGfxQueryPool*)queries;
	if (dsGLResource_destroy(&glQueries->resource))
		return destroyImpl(queries);

	return true;
}

void dsGLGfxQueryPool_addInternalRef(dsGfxQueryPool* queries)
{
	DS_ASSERT(queries);
	dsGLGfxQueryPool* glQueries = (dsGLGfxQueryPool*)queries;
	dsGLResource_addRef(&glQueries->resource);
}

void dsGLGfxQueryPool_freeInternalRef(dsGfxQueryPool* queries)
{
	DS_ASSERT(queries);
	dsGLGfxQueryPool* glQueries = (dsGLGfxQueryPool*)queries;
	if (dsGLResource_freeRef(&glQueries->resource))
		destroyImpl(queries);
}
