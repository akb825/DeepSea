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

#include "Resources/GLShaderModule.h"

#include "Resources/GLResource.h"
#include "Types.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>

dsShaderModule* dsGLShaderModule_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	mslModule* module)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(allocator);

	dsGLShaderModule* shaderModule = (dsGLShaderModule*)dsAllocator_alloc(allocator,
		sizeof(dsGLShaderModule));
	if (!shaderModule)
		return NULL;

	dsShaderModule* baseShaderModule = (dsShaderModule*)shaderModule;
	baseShaderModule->resourceManager = resourceManager;
	baseShaderModule->allocator = dsAllocator_keepPointer(allocator);
	baseShaderModule->module = module;

	dsGLResource_initialize(&shaderModule->resource);
	return baseShaderModule;
}

static bool destroyImpl(dsShaderModule* module)
{
	if (module->allocator)
		return dsAllocator_free(module->allocator, module);

	return true;
}

bool dsGLShaderModule_destroy(dsResourceManager* resourceManager, dsShaderModule* module)
{
	DS_UNUSED(resourceManager);
	DS_ASSERT(module);

	dsGLShaderModule* glModule = (dsGLShaderModule*)module;
	if (dsGLResource_destroy(&glModule->resource))
		return destroyImpl(module);

	return true;
}

void dsGLShaderModule_addInternalRef(dsShaderModule* module)
{
	DS_ASSERT(module);
	dsGLShaderModule* glModule = (dsGLShaderModule*)module;
	dsGLResource_addRef(&glModule->resource);
}

void dsGLShaderModule_freeInternalRef(dsShaderModule* module)
{
	DS_ASSERT(module);
	dsGLShaderModule* glModule = (dsGLShaderModule*)module;
	if (dsGLResource_freeRef(&glModule->resource))
		destroyImpl(module);
}
