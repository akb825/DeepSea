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

dsMaterialDesc* dsVkMaterialDesc_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	const dsMaterialElement* elements, uint32_t elementCount);
bool dsVkMaterialDesc_destroy(dsResourceManager* resourceManager, dsMaterialDesc* materialDesc);

void dsVkMaterialDesc_initializeBindings(const dsMaterialDesc* materialDesc,
	dsVkBindingMemory* bindingMemory, dsMaterialBinding binding);
dsVkMaterialDescriptor* dsVkMaterialDesc_createDescriptor(const dsMaterialDesc* materialDesc,
	dsAllocator* allocator, dsMaterialBinding binding);
void dsVkMaterialDesc_freeDescriptor(const dsMaterialDesc* materialDesc,
	dsVkMaterialDescriptor* descriptor);
