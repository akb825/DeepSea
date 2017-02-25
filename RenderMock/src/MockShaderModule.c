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

#include "MockShaderModule.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>

dsShaderModule* dsMockShaderModule_create(dsResourceManager* resourceManager,
	dsAllocator* allocator, mslModule* module)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(allocator);
	DS_ASSERT(module);

	dsShaderModule* shaderModule = (dsShaderModule*)dsAllocator_alloc(allocator,
		sizeof(dsShaderModule));
	if (!shaderModule)
		return NULL;

	shaderModule->resourceManager = resourceManager;
	if (allocator->freeFunc)
		shaderModule->allocator = allocator;
	else
		shaderModule->allocator = NULL;
	shaderModule->module = module;

	return shaderModule;
}

bool dsMockShaderModule_destroy(dsResourceManager* resourceManager, dsShaderModule* module)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(module);
	DS_UNUSED(resourceManager);

	if (module->allocator)
		return dsAllocator_free(module->allocator, module);
	else
		return true;
}
