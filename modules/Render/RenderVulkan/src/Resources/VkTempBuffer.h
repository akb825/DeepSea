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

#include "VkTypes.h"

dsVkTempBuffer* dsVkTempBuffer_create(dsAllocator* allocator, dsVkDevice* device, size_t size);
void* dsVkTempBuffer_allocate(size_t* outOffset, dsVkTempBuffer* buffer, size_t size,
	uint32_t alignment);
bool dsVkTempBuffer_reset(dsVkTempBuffer* buffer, uint64_t finishedSubmitCount);
void dsVkTempBuffer_destroy(dsVkTempBuffer* buffer);
