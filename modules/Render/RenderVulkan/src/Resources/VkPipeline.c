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

#include "Resources/VkPipeline.h"
#include "VkShared.h"
#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/Lifetime.h>
#include <string.h>

uint32_t dsVkPipeline_hash(uint32_t samples, uint32_t defaultAnisotropy,
	dsPrimitiveType primitiveType, dsVertexFormat formats[DS_MAX_GEOMETRY_VERTEX_BUFFERS],
	dsRenderPass* renderPass, uint32_t subpass)
{
	uint32_t hash = dsHash32(&samples);
	hash = dsHashCombine(hash, dsHash32(&defaultAnisotropy));
	uint32_t primitiveType32 = primitiveType;
	hash = dsHashCombine(hash, dsHash32(&primitiveType32));
	hash = dsHashCombineBytes(hash, formats, sizeof(dsVertexFormat)*DS_MAX_GEOMETRY_VERTEX_BUFFERS);
	hash = dsHashCombine(hash, dsHashPointer(renderPass));
	return dsHashCombine(hash, dsHash32(&subpass));
}

/*dsVkPipeline* dsVkPipeline_create(dsAllocator* allocator, dsShader* shader, const uint32_t* spirv,
	uint32_t spirvCount, VkPipeline existingPipeline, uint32_t hash, uint32_t samples,
	uint32_t defaultAnisotropy, dsPrimitiveType primitiveType,
	dsVertexFormat formats[DS_MAX_GEOMETRY_VERTEX_BUFFERS],
	dsLifetime* renderPass, uint32_t subpass)
{
}*/

bool dsVkPipeline_isEquivalent(const dsVkPipeline* pipeline, uint32_t hash, uint32_t samples,
	uint32_t defaultAnisotropy, dsPrimitiveType primitiveType,
	dsVertexFormat formats[DS_MAX_GEOMETRY_VERTEX_BUFFERS],
	dsRenderPass* renderPass, uint32_t subpass)
{
	if (pipeline->hash != hash)
		return false;

	return pipeline->samples == samples && pipeline->defaultAnisotropy == defaultAnisotropy &&
		pipeline->primitiveType == primitiveType && pipeline->subpass == subpass &&
		memcmp(pipeline->formats, formats,
			sizeof(dsVertexFormat)*DS_MAX_GEOMETRY_VERTEX_BUFFERS) == 0 &&
		dsLifetime_getObject(pipeline->renderPass) == renderPass;
}
