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

#include <DeepSea/Core/Config.h>

#include "Resources/VkDrawGeometry.h"
#include "VkTypes.h"
#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <string.h>

dsDrawGeometry* dsVkDrawGeometry_create(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsVertexBuffer* vertexBuffers[DS_MAX_GEOMETRY_VERTEX_BUFFERS],
	dsIndexBuffer* indexBuffer)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(allocator);
	dsVkDrawGeometry* geometry = DS_ALLOCATE_OBJECT(allocator, dsVkDrawGeometry);
	if (!geometry)
		return NULL;

	dsDrawGeometry* baseGeometry = (dsDrawGeometry*)geometry;
	baseGeometry->resourceManager = resourceManager;
	baseGeometry->allocator = dsAllocator_keepPointer(allocator);

	geometry->vertexHash = 0;
	for (unsigned int i = 0; i < DS_MAX_GEOMETRY_VERTEX_BUFFERS; ++i)
	{
		if (vertexBuffers[i])
			baseGeometry->vertexBuffers[i] = *vertexBuffers[i];
		else
			memset(baseGeometry->vertexBuffers + i, 0, sizeof(*baseGeometry->vertexBuffers));

		dsVertexFormat* format = &baseGeometry->vertexBuffers[i].format;
		geometry->vertexHash = dsHashCombineBytes(geometry->vertexHash, format, sizeof(*format));
	}

	if (indexBuffer)
		baseGeometry->indexBuffer = *indexBuffer;
	else
		memset(&baseGeometry->indexBuffer, 0, sizeof(baseGeometry->indexBuffer));

	return baseGeometry;
}

bool dsVkDrawGeometry_destroy(dsResourceManager* resourceManager, dsDrawGeometry* geometry)
{
	DS_UNUSED(resourceManager);
	if (geometry->allocator)
		DS_VERIFY(dsAllocator_free(geometry->allocator, geometry));
	return true;
}
