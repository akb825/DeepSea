/*
 * Copyright 2020 Aaron Barany
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

#include <DeepSea/VectorDrawScene/VectorSceneShaders.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/VectorDraw/VectorShaderModule.h>
#include <DeepSea/VectorDraw/VectorShaders.h>

#include <string.h>

bool dsVectorSceneShaders_destroy(void* customResource)
{
	if (!customResource)
		return true;

	dsVectorShaders* shaders = (dsVectorShaders*)customResource;
	dsVectorShaderModule* shaderModule = shaders->shaderModule;
	if (!dsVectorShaders_destroy(shaders))
		return false;

	DS_VERIFY(dsVectorShaderModule_destroy(shaderModule));
	return true;
}

const char* const dsVectorSceneShaders_typeName = "VectorShaderBundle";

static dsCustomSceneResourceType resourceType;
const dsCustomSceneResourceType* dsVectorSceneShaders_type(void)
{
	return &resourceType;
}

dsCustomSceneResource* dsVectorSceneShaders_create(dsAllocator* allocator, dsVectorShaders* shaders)
{
	if (!allocator || !shaders)
	{
		errno = EINVAL;
		DS_VERIFY(dsVectorSceneShaders_destroy(shaders));
		return NULL;
	}

	dsCustomSceneResource* customResource = DS_ALLOCATE_OBJECT(allocator, dsCustomSceneResource);
	if (!customResource)
	{
		DS_VERIFY(dsVectorSceneShaders_destroy(shaders));
		return NULL;
	}

	customResource->allocator = dsAllocator_keepPointer(allocator);
	customResource->type = &resourceType;
	customResource->resource = shaders;
	customResource->destroyFunc = &dsVectorSceneShaders_destroy;
	return customResource;
}

