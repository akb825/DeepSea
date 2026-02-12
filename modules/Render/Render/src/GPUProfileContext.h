/*
 * Copyright 2018-2026 Aaron Barany
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
#include <DeepSea/Render/Types.h>

dsGPUProfileContext* dsGPUProfileContext_create(
	dsResourceManager* resourceManager, dsAllocator* allocator);

void dsGPUProfileContext_beginFrame(dsGPUProfileContext* context);
void dsGPUProfileContext_endFrame(dsGPUProfileContext* context);

void dsGPUProfileContext_beginDeferredResources(dsGPUProfileContext* context);
void dsGPUProfileContext_endDeferredResources(dsGPUProfileContext* context);

void dsGPUProfileContext_beginSwapBuffers(dsGPUProfileContext* context);
void dsGPUProfileContext_endSwapBuffers(dsGPUProfileContext* context);

void dsGPUProfileContext_beginSurface(
	dsGPUProfileContext* context, dsCommandBuffer* commandBuffer, const char* surfaceName);
void dsGPUProfileContext_endSurface(dsGPUProfileContext* context, dsCommandBuffer* commandBuffer);

void dsGPUProfileContext_beginSubpass(dsGPUProfileContext* context, dsCommandBuffer* commandBuffer,
	const char* framebufferName, const char* subpassName, bool secondary);
void dsGPUProfileContext_nextSubpass(
	dsGPUProfileContext* context, dsCommandBuffer* commandBuffer, const char* subpassName,
	bool secondary);
void dsGPUProfileContext_endSubpass(dsGPUProfileContext* context, dsCommandBuffer* commandBuffer);

void dsGPUProfileContext_beginCompute(dsGPUProfileContext* context, dsCommandBuffer* commandBuffer,
	const char* moduleName, const char* shaderName);
void dsGPUProfileContext_endCompute(dsGPUProfileContext* context, dsCommandBuffer* commandBuffer);

void dsGPUProfileContext_destroy(dsGPUProfileContext* context);
