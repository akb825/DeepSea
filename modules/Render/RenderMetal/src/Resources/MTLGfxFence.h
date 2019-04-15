/*
 * Copyright 2019 Aaron Barany
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
#include "MTLTypes.h"

dsGfxFence* dsMTLGfxFence_create(dsResourceManager* resourceManager, dsAllocator* allocator);
bool dsMTLGfxFence_set(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsGfxFence** fences, uint32_t fenceCount, bool bufferReadback);
dsGfxFenceResult dsMTLGfxFence_wait(dsResourceManager* resourceManager, dsGfxFence* fence,
	uint64_t timeout);
bool dsMTLGfxFence_reset(dsResourceManager* resourceManager, dsGfxFence* fence);
bool dsMTLGfxFence_destroy(dsResourceManager* resourceManager, dsGfxFence* fence);
