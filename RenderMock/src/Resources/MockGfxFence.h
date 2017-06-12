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

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Render/Resources/Types.h>

dsGfxFence* dsMockGfxFence_create(dsResourceManager* resourceManager, dsAllocator* allocator);
bool dsMockGfxFence_set(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsGfxFence** fences, uint32_t fenceCount, bool bufferReadback);
dsGfxFenceResult dsMockGfxFence_wait(dsResourceManager* resourceManager, dsGfxFence* fence,
	uint64_t timeout);
bool dsMockGfxFence_reset(dsResourceManager* resourceManager, dsGfxFence* fence);
bool dsMockGfxFence_destroy(dsResourceManager* resourceManager, dsGfxFence* fence);
