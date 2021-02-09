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

#include "Resources/VkShaderModule.h"
#include "VkShared.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Log.h>

#include <MSL/Client/ModuleC.h>
#include <string.h>

dsShaderModule* dsVkShaderModule_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	mslModule* module, const char* name)
{
	size_t nameLen = strlen(name) + 1;
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsShaderModule)) + DS_ALIGNED_SIZE(nameLen);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsShaderModule* shaderModule = DS_ALLOCATE_OBJECT(&bufferAlloc, dsShaderModule);
	DS_ASSERT(shaderModule);

	shaderModule->resourceManager = resourceManager;
	shaderModule->allocator = dsAllocator_keepPointer(allocator);
	shaderModule->module = module;

	char* nameCopy = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
	DS_ASSERT(nameCopy);
	memcpy(nameCopy, name, nameLen);
	shaderModule->name = nameCopy;

	return shaderModule;
}

bool dsVkShaderModule_destroy(dsResourceManager* resourceManager, dsShaderModule* module)
{
	DS_UNUSED(resourceManager);
	if (module->allocator)
		DS_VERIFY(dsAllocator_free(module->allocator, module));
	return true;
}
