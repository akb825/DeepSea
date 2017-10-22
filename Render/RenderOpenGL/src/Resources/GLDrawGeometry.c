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

#include "Resources/GLDrawGeometry.h"

#include "AnyGL/gl.h"
#include "Resources/GLGfxBuffer.h"
#include "Resources/GLResourceManager.h"
#include "Resources/GLResource.h"
#include "GLRendererInternal.h"
#include "Types.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Bits.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Render/Resources/VertexFormat.h>
#include <string.h>

static void bindElements(dsGLDrawGeometry* geometry, uint32_t baseVertex, bool track)
{
	dsDrawGeometry* baseGeometry = (dsDrawGeometry*)geometry;
	dsResourceManager* resourceManager = baseGeometry->resourceManager;
	dsGLRenderer* renderer = (dsGLRenderer*)resourceManager->renderer;
	bool enabled[DS_MAX_ALLOWED_VERTEX_ATTRIBS];
	memset(enabled, 0, sizeof(enabled));

	for (unsigned int i = 0; i < DS_MAX_GEOMETRY_VERTEX_BUFFERS; ++i)
	{
		dsVertexBuffer* vertexBuffer = baseGeometry->vertexBuffers + i;
		if (!vertexBuffer->buffer)
			continue;

		size_t offset = baseVertex*vertexBuffer->format.size;
		dsGLGfxBuffer* glBuffer = (dsGLGfxBuffer*)vertexBuffer->buffer;
		glBindBuffer(GL_ARRAY_BUFFER, glBuffer->bufferId);
		for (uint32_t mask = vertexBuffer->format.enabledMask; mask;
			mask = dsRemoveLastBit(mask))
		{
			uint32_t index = dsBitmaskIndex(mask);
			DS_ASSERT(index < resourceManager->maxVertexAttribs &&
				index < DS_MAX_ALLOWED_VERTEX_ATTRIBS);
			const dsVertexElement* element = vertexBuffer->format.elements + index;

			GLenum type = 0;
			GLint elements = 0;
			bool normalized = false;
			// Format should have been pre-validated.
			DS_VERIFY(dsGLResourceManager_getVertexFormatInfo(&type, &elements,
				&normalized, resourceManager, element->format));

			enabled[index] = true;
			if (!track || !renderer->boundAttributes[index])
				glEnableVertexAttribArray(index);
			glVertexAttribPointer(index, elements, type, normalized,
				vertexBuffer->format.size, (void*)(size_t)(vertexBuffer->offset + element->offset +
				offset));
			if (ANYGL_SUPPORTED(glVertexAttribDivisor))
				glVertexAttribDivisor(index, vertexBuffer->format.divisor);
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	if (baseGeometry->indexBuffer.buffer)
	{
		dsGLGfxBuffer* glBuffer = (dsGLGfxBuffer*)baseGeometry->indexBuffer.buffer;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glBuffer->bufferId);
	}

	if (track)
	{
		for (unsigned int i = 0; i < resourceManager->maxVertexAttribs; ++i)
		{
			if (!enabled[i] && renderer->boundAttributes[i])
				glDisableVertexAttribArray(i);
		}
		memcpy(renderer->boundAttributes, enabled, sizeof(renderer->boundAttributes));
	}
}

dsDrawGeometry* dsGLDrawGeometry_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsVertexBuffer* vertexBuffers[DS_MAX_GEOMETRY_VERTEX_BUFFERS], dsIndexBuffer* indexBuffer)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(allocator);

	dsGLDrawGeometry* geometry = (dsGLDrawGeometry*)dsAllocator_alloc(allocator,
		sizeof(dsGLDrawGeometry));
	if (!geometry)
		return NULL;

	dsDrawGeometry* baseGeometry = (dsDrawGeometry*)geometry;
	baseGeometry->resourceManager = resourceManager;
	baseGeometry->allocator = dsAllocator_keepPointer(allocator);
	for (unsigned int i = 0; i < DS_MAX_GEOMETRY_VERTEX_BUFFERS; ++i)
	{
		if (vertexBuffers[i])
			baseGeometry->vertexBuffers[i] = *vertexBuffers[i];
		else
			memset(baseGeometry->vertexBuffers + i, 0, sizeof(dsVertexBuffer));
	}
	if (indexBuffer)
		memcpy(&baseGeometry->indexBuffer, indexBuffer, sizeof(dsIndexBuffer));
	else
		memset(&baseGeometry->indexBuffer, 0, sizeof(dsIndexBuffer));

	dsGLResource_initialize(&geometry->resource);
	geometry->vao = 0;
	geometry->vaoContext = 0;

	return baseGeometry;
}

static bool destroyImpl(dsDrawGeometry* geometry)
{
	dsGLDrawGeometry* glGeometry = (dsGLDrawGeometry*)geometry;
	dsGLRenderer_destroyVao(geometry->resourceManager->renderer, glGeometry->vao,
		glGeometry->vaoContext);

	if (geometry->allocator)
		return dsAllocator_free(geometry->allocator, geometry);

	return true;
}

bool dsGLDrawGeometry_destroy(dsResourceManager* resourceManager, dsDrawGeometry* geometry)
{
	DS_UNUSED(resourceManager);
	DS_ASSERT(geometry);

	dsGLDrawGeometry* glBuffer = (dsGLDrawGeometry*)geometry;
	if (dsGLResource_destroy(&glBuffer->resource))
		return destroyImpl(geometry);

	return true;
}

void dsGLDrawGeometry_bind(const dsDrawGeometry* geometry, int32_t baseVertex)
{
	dsGLDrawGeometry* glGeometry = (dsGLDrawGeometry*)geometry;;
	if (ANYGL_SUPPORTED(glGenVertexArrays))
	{
		DS_ASSERT(baseVertex == 0);

		// Vertex array objects are tied to specific contexts.
		dsGLRenderer* renderer = (dsGLRenderer*)geometry->resourceManager->renderer;
		if (!glGeometry->vao || glGeometry->vaoContext != renderer->contextCount)
		{
			glGenVertexArrays(1, &glGeometry->vao);
			glBindVertexArray(glGeometry->vao);
			bindElements(glGeometry, baseVertex, false);
		}
		else
			glBindVertexArray(glGeometry->vao);
	}
	else
		bindElements(glGeometry, baseVertex, true);
}

void dsGLDrawGeometry_addInternalRef(dsDrawGeometry* geometry)
{
	DS_ASSERT(geometry);
	dsGLDrawGeometry* glBuffer = (dsGLDrawGeometry*)geometry;
	dsGLResource_addRef(&glBuffer->resource);

	for (unsigned int i = 0; i < DS_MAX_GEOMETRY_VERTEX_BUFFERS; ++i)
	{
		if (geometry->vertexBuffers[i].buffer)
			dsGLGfxBuffer_addInternalRef(geometry->vertexBuffers[i].buffer);
	}
	if (geometry->indexBuffer.buffer)
		dsGLGfxBuffer_addInternalRef(geometry->indexBuffer.buffer);
}

void dsGLDrawGeometry_freeInternalRef(dsDrawGeometry* geometry)
{
	DS_ASSERT(geometry);
	for (unsigned int i = 0; i < DS_MAX_GEOMETRY_VERTEX_BUFFERS; ++i)
	{
		if (geometry->vertexBuffers[i].buffer)
			dsGLGfxBuffer_freeInternalRef(geometry->vertexBuffers[i].buffer);
	}
	if (geometry->indexBuffer.buffer)
		dsGLGfxBuffer_freeInternalRef(geometry->indexBuffer.buffer);

	dsGLDrawGeometry* glBuffer = (dsGLDrawGeometry*)geometry;
	if (dsGLResource_freeRef(&glBuffer->resource))
		destroyImpl(geometry);
}
