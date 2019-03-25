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

void dsVkSubpassBuffers_initialize(dsVkSubpassBuffers* buffers, dsAllocator* allocator);
bool dsVkSubpassBuffers_addSubpass(dsVkSubpassBuffers* buffers);
bool dsVkSubpassBuffers_addCommandBuffer(dsVkSubpassBuffers* buffers, VkCommandBuffer commandBuffer);
void dsVkSubpassBuffers_reset(dsVkSubpassBuffers* buffers);
void dsVkSubpassBuffers_shutdown(dsVkSubpassBuffers* buffers);
