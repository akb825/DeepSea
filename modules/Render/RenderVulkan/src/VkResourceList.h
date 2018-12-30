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
#include "VkTypes.h"

void dsVkResourceList_initialize(dsVkResourceList* resources, dsAllocator* allocator);

bool dsVkResourceList_addBuffer(dsVkResourceList* resources, dsVkGfxBufferData* buffer);
bool dsVkResourceList_addTexture(dsVkResourceList* resources, dsTexture* texture);
bool dsVkResourceList_addCopyImage(dsVkResourceList* resources, dsVkCopyImage* image);
bool dsVkResourceList_addRenderbuffer(dsVkResourceList* resources, dsRenderbuffer* renderbuffer);
bool dsVkResourceList_addFramebuffer(dsVkResourceList* resources, dsVkRealFramebuffer* framebuffer);
bool dsVkResourceList_addFence(dsVkResourceList* resources, dsGfxFence* fence);
bool dsVkResourceList_addQueries(dsVkResourceList* resources, dsGfxQueryPool* queries);
bool dsVkResourceList_addMaterialDescriptor(dsVkResourceList* resources,
	dsVkMaterialDescriptor* descriptor);
bool dsVkResourceList_addSamplerList(dsVkResourceList* resources, dsVkSamplerList* samplers);
bool dsVkResourceList_addComputePipeline(dsVkResourceList* resources,
	dsVkComputePipeline* pipeline);
bool dsVkResourceList_addPipeline(dsVkResourceList* resources, dsVkPipeline* pipeline);
bool dsVkResourceList_addRenderSurface(dsVkResourceList* resources, dsVkRenderSurfaceData* surface);
void dsVkResourceList_clear(dsVkResourceList* resources);

void dsVkResourceList_shutdown(dsVkResourceList* resources);
