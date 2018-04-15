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

#include "Resources/MockDrawGeometry.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <string.h>

dsDrawGeometry* dsMockDrawGeometry_create(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsVertexBuffer* vertexBuffers[DS_MAX_GEOMETRY_VERTEX_BUFFERS],
	dsIndexBuffer* indexBuffer)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(allocator);
	dsDrawGeometry* geometry = DS_ALLOCATE_OBJECT(allocator, dsDrawGeometry);
	if (!geometry)
		return NULL;

	geometry->resourceManager = resourceManager;
	geometry->allocator = dsAllocator_keepPointer(allocator);

	for (unsigned int i = 0; i < DS_MAX_GEOMETRY_VERTEX_BUFFERS; ++i)
	{
		if (vertexBuffers[i])
			geometry->vertexBuffers[i] = *vertexBuffers[i];
		else
			memset(geometry->vertexBuffers + i, 0, sizeof(*geometry->vertexBuffers));
	}

	if (indexBuffer)
		geometry->indexBuffer = *indexBuffer;
	else
		memset(&geometry->indexBuffer, 0, sizeof(geometry->indexBuffer));

	return geometry;
}

bool dsMockDrawGeometry_destroy(dsResourceManager* resourceManager, dsDrawGeometry* geometry)
{
	DS_UNUSED(resourceManager);
	if (geometry->allocator)
		return dsAllocator_free(geometry->allocator, geometry);
	return true;
}
