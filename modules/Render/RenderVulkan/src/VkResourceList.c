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

#include "VkResourceList.h"
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <string.h>

void dsVkResourceList_initialize(dsVkResourceList* resources, dsAllocator* allocator)
{
	memset(resources, 0, sizeof(*resources));
	DS_ASSERT(allocator->freeFunc);
	resources->allocator = allocator;
}

bool dsVkResourceList_addBuffer(dsVkResourceList* resources, dsVkGfxBufferData* buffer)
{
	uint32_t index = resources->bufferCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(resources->allocator, resources->buffers, resources->bufferCount,
		resources->maxBuffers, 1))
	{
		return false;
	}

	resources->buffers[index] = buffer;
	return true;
}

bool dsVkResourceList_addTexture(dsVkResourceList* resources, dsTexture* texture)
{
	uint32_t index = resources->textureCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(resources->allocator, resources->textures, resources->textureCount,
		resources->maxTextures, 1))
	{
		return false;
	}

	resources->textures[index] = texture;
	return true;
}

bool dsVkResourceList_addCopyImage(dsVkResourceList* resources, dsVkCopyImage* image)
{
	uint32_t index = resources->copyImageCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(resources->allocator, resources->copyImages,
		resources->copyImageCount, resources->maxCopyImages, 1))
	{
		return false;
	}

	resources->copyImages[index] = image;
	return true;
}

bool dsVkResourceList_addRenderbuffer(dsVkResourceList* resources, dsRenderbuffer* renderbuffer)
{
	uint32_t index = resources->renderbufferCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(resources->allocator, resources->renderbuffers,
		resources->renderbufferCount, resources->maxRenderbuffers, 1))
	{
		return false;
	}

	resources->renderbuffers[index] = renderbuffer;
	return true;
}

bool dsVkResourceList_addFramebuffer(dsVkResourceList* resources, dsVkRealFramebuffer* framebuffer)
{
	uint32_t index = resources->framebufferCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(resources->allocator, resources->framebuffers,
		resources->framebufferCount, resources->maxFramebuffers, 1))
	{
		return false;
	}

	resources->framebuffers[index] = framebuffer;
	return true;
}

bool dsVkResourceList_addFence(dsVkResourceList* resources, dsGfxFence* fence)
{
	uint32_t index = resources->fenceCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(resources->allocator, resources->fences, resources->fenceCount,
		resources->maxFences, 1))
	{
		return false;
	}

	resources->fences[index] = fence;
	return true;
}

bool dsVkResourceList_addQueries(dsVkResourceList* resources, dsGfxQueryPool* queries)
{
	uint32_t index = resources->queryCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(resources->allocator, resources->queries, resources->queryCount,
		resources->maxQueries, 1))
	{
		return false;
	}

	resources->queries[index] = queries;
	return true;
}

bool dsVkResourceList_addMaterialDescriptor(dsVkResourceList* resources,
	dsVkMaterialDescriptor* descriptor)
{
	uint32_t index = resources->descriptorCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(resources->allocator, resources->descriptors,
		resources->descriptorCount, resources->maxDescriptors, 1))
	{
		return false;
	}

	resources->descriptors[index] = descriptor;
	return true;
}

bool dsVkResourceList_addSamplerList(dsVkResourceList* resources, dsVkSamplerList* samplers)
{
	uint32_t index = resources->samplerCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(resources->allocator, resources->samplers,
		resources->samplerCount, resources->maxSamplers, 1))
	{
		return false;
	}

	resources->samplers[index] = samplers;
	return true;
}

bool dsVkResourceList_addComputePipeline(dsVkResourceList* resources, dsVkComputePipeline* pipeline)
{
	uint32_t index = resources->computePipelineCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(resources->allocator, resources->computePipelines,
		resources->computePipelineCount, resources->maxComputePipelines, 1))
	{
		return false;
	}

	resources->computePipelines[index] = pipeline;
	return true;
}

bool dsVkResourceList_addPipeline(dsVkResourceList* resources, dsVkPipeline* pipeline)
{
	uint32_t index = resources->pipelineCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(resources->allocator, resources->pipelines,
		resources->pipelineCount, resources->maxPipelines, 1))
	{
		return false;
	}

	resources->pipelines[index] = pipeline;
	return true;
}

bool dsVkResourceList_addRenderSurface(dsVkResourceList* resources, dsVkRenderSurfaceData* surface)
{
	uint32_t index = resources->renderSurfaceCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(resources->allocator, resources->renderSurfaces,
		resources->renderSurfaceCount, resources->maxRenderSurfaces, 1))
	{
		return false;
	}

	resources->renderSurfaces[index] = surface;
	return true;
}

void dsVkResourceList_clear(dsVkResourceList* resources)
{
	resources->bufferCount = 0;
	resources->textureCount = 0;
	resources->copyImageCount = 0;
	resources->renderbufferCount = 0;
	resources->framebufferCount = 0;
	resources->fenceCount = 0;
	resources->queryCount = 0;
	resources->descriptorCount = 0;
	resources->samplerCount = 0;
	resources->computePipelineCount = 0;
	resources->pipelineCount = 0;
	resources->renderSurfaceCount = 0;
}

void dsVkResourceList_shutdown(dsVkResourceList* resources)
{
	DS_VERIFY(dsAllocator_free(resources->allocator, resources->buffers));
	DS_VERIFY(dsAllocator_free(resources->allocator, resources->textures));
	DS_VERIFY(dsAllocator_free(resources->allocator, resources->copyImages));
	DS_VERIFY(dsAllocator_free(resources->allocator, resources->renderbuffers));
	DS_VERIFY(dsAllocator_free(resources->allocator, resources->framebuffers));
	DS_VERIFY(dsAllocator_free(resources->allocator, resources->fences));
	DS_VERIFY(dsAllocator_free(resources->allocator, resources->queries));
	DS_VERIFY(dsAllocator_free(resources->allocator, resources->descriptors));
	DS_VERIFY(dsAllocator_free(resources->allocator, resources->samplers));
	DS_VERIFY(dsAllocator_free(resources->allocator, resources->computePipelines));
	DS_VERIFY(dsAllocator_free(resources->allocator, resources->pipelines));
	DS_VERIFY(dsAllocator_free(resources->allocator, resources->renderSurfaces));
}
