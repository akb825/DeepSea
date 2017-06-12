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
#include "Types.h"

dsGfxFence* dsGLGfxFence_create(dsResourceManager* resourceManager, dsAllocator* allocator);
bool dsGLGfxFence_set(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsGfxFence** fencees, uint32_t fenceCount, bool bufferReadback);
dsGfxFenceResult dsGLGfxFence_wait(dsResourceManager* resourceManager, dsGfxFence* fence,
	uint64_t timeout);
bool dsGLGfxFence_reset(dsResourceManager* resourceManager, dsGfxFence* fence);
bool dsGLGfxFence_destroy(dsResourceManager* resourceManager, dsGfxFence* fence);

void dsGLFenceSync_addRef(dsGLFenceSync* sync);
void dsGLFenceSync_freeRef(dsGLFenceSync* sync);
void dsGLFenceSyncRef_addRef(dsGLFenceSyncRef* sync);
void dsGLFenceSyncRef_freeRef(dsGLFenceSyncRef* sync);
