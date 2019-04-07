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

bool dsMTLCommandBuffer_initialize(dsMTLCommandBuffer* commandBuffer, dsRenderer* renderer,
	dsAllocator* allocator, dsCommandBufferUsage usage);

id<MTLCommandBuffer> dsMTLCommandBuffer_getCommandBuffer(dsCommandBuffer* commandBuffer);
id<MTLBlitCommandEncoder> dsMTLCommandBuffer_getBlitCommandEncoder(dsCommandBuffer* commandBuffer);
id<MTLComputeCommandEncoder> dsMTLCommandBuffer_getComputeCommandEncoder(
	dsCommandBuffer* commandBuffer);

bool dsMTLCommandBuffer_addGfxBuffer(dsCommandBuffer* commandBuffer, dsMTLGfxBufferData* buffer);

void dsMTLCommandBuffer_shutdown(dsMTLCommandBuffer* commandBuffer);
