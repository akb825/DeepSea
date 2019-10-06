/*
 * Copyright 2018-2019 Aaron Barany
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

dsVkMaterialDescriptor* dsVkMaterialDescriptor_create(dsRenderer* renderer, dsAllocator* allocator,
	const dsMaterialDesc* materialDesc, const dsVkBindingCounts* counts, dsMaterialBinding binding);
bool dsVkMaterialDescriptor_shouldCheckPointers(const dsVkMaterialDescriptor* descriptor,
	const dsVkSamplerList* samplers, const void* refObject, uint32_t pointerVersion);
bool dsVkMaterialDescriptor_shouldCheckOffsets(const dsVkMaterialDescriptor* descriptor,
	uint32_t offsetVersion);
bool dsVkMaterialDescriptor_isUpToDate(const dsVkMaterialDescriptor* descriptor,
	const dsVkBindingMemory* bindingMemory);
void dsVkMaterialDescriptor_updateEarlyChecks(dsVkMaterialDescriptor* descriptor,
	const dsVkSamplerList* samplers, const void* refObject, uint32_t pointerVersion,
	uint32_t offsetVersion);
void dsVkMaterialDescriptor_update(dsVkMaterialDescriptor* descriptor, const dsShader* shader,
	dsVkBindingMemory* bindingMemory, const dsVkSamplerList* samplers, const void* refObject,
	uint32_t pointerVersion, uint32_t offsetVersion);
void dsVkMaterialDescriptor_destroy(dsVkMaterialDescriptor* descriptor);
