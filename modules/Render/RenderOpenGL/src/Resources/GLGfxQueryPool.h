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

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Render/Resources/Types.h>

dsGfxQueryPool* dsGLGfxQueryPool_create(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsGfxQueryType type, uint32_t count);
bool dsGLGfxQueryPool_reset(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsGfxQueryPool* queries, uint32_t first, uint32_t count);
bool dsGLGfxQueryPool_beginQuery(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsGfxQueryPool* queries, uint32_t query);
bool dsGLGfxQueryPool_endQuery(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsGfxQueryPool* queries, uint32_t query);
bool dsGLGfxQueryPool_queryTimestamp(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, dsGfxQueryPool* queries, uint32_t query);
bool dsGLGfxQueryPool_getValues(dsResourceManager* resourceManager, dsGfxQueryPool* queries,
	uint32_t first, uint32_t count, void* data, size_t dataSize, size_t stride, size_t elementSize,
	bool checkAvailability);
bool dsGLGfxQueryPool_copyValues(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsGfxQueryPool* queries, uint32_t first, uint32_t count, dsGfxBuffer* buffer, size_t offset,
	size_t stride, size_t elementSize, bool checkAvailability);
bool dsGLGfxQueryPool_destroy(dsResourceManager* resourceManager,
	dsGfxQueryPool* queries);

void dsGLGfxQueryPool_addInternalRef(dsGfxQueryPool* queries);
void dsGLGfxQueryPool_freeInternalRef(dsGfxQueryPool* queries);
