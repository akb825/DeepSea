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

#include <DeepSea/VectorDraw/VectorShaders.h>

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Render/Resources/Shader.h>
#include <DeepSea/Render/Resources/ShaderModule.h>
#include <string.h>

extern const char* dsDefaultVectorShaderNames[dsVectorShaderType_Count];

static dsVectorShaders* dsVectorShaders_createImpl(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsVectorShaderModule* shaderModule,
	uint32_t shaderIndices[dsVectorShaderType_Count],
	const char* shaderNames[dsVectorShaderType_Count])
{
	if (!resourceManager || (!allocator && !resourceManager->allocator) || !shaderModule)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator)
		allocator = resourceManager->allocator;

	for (uint32_t i = 0; i < (uint32_t)dsVectorShaderType_Count; ++i)
	{
		if (shaderIndices[i] == DS_MATERIAL_UNKNOWN)
		{
			errno = ENOTFOUND;
			DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG,
				"Vector shader module doesn't contain shader '%s'.", shaderNames[i]);
			return NULL;
		}
	}

	bool textHasTessellation = dsShaderModule_shaderIndexHasStage(shaderModule->shaderModule,
		shaderModule->shaderIndices[dsVectorShaderType_TextColor],
		dsShaderStage_TessellationEvaluation);
	for (uint32_t i = dsVectorShaderType_TextColorOutline;
		i <= dsVectorShaderType_TextGradientOutline; ++i)
	{
		if (dsShaderModule_shaderIndexHasStage(shaderModule->shaderModule,
			shaderModule->shaderIndices[i],
			dsShaderStage_TessellationEvaluation) != textHasTessellation)
		{
			errno = EPERM;
			DS_LOG_ERROR(DS_VECTOR_DRAW_LOG_TAG,
				"Cannot have a mixture of text shaders with and without tessellation.");
			return NULL;
		}
	}

	dsVectorShaders* shaders = DS_ALLOCATE_OBJECT(allocator, dsVectorShaders);
	if (!shaders)
		return NULL;

	shaders->allocator = dsAllocator_keepPointer(allocator);
	shaders->shaderModule = shaderModule;

	for (uint32_t i = 0; i < (uint32_t)dsVectorShaderType_Count; ++i)
	{
		shaders->shaders[i] = dsShader_createIndex(resourceManager, allocator,
			shaderModule->shaderModule, shaderIndices[i], shaderModule->materialDesc);
		if (!shaders->shaders[i])
		{
			for (uint32_t j = 0; j < i; ++j)
				DS_VERIFY(dsShader_destroy(shaders->shaders[j]));

			if (shaders->allocator)
				DS_VERIFY(dsAllocator_free(allocator, shaders));
			return NULL;
		}
	}

	return shaders;
}

dsVectorShaders* dsVectorShaders_create(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsVectorShaderModule* shaderModule)
{
	return dsVectorShaders_createImpl(resourceManager, allocator, shaderModule,
		shaderModule->shaderIndices, dsDefaultVectorShaderNames);
}

dsVectorShaders* dsVectorShaders_createCustom(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsVectorShaderModule* shaderModule,
	const char* shaderNames[dsVectorShaderType_Count])
{
	if (!resourceManager || (!allocator && !resourceManager->allocator) || !shaderModule)
	{
		errno = EINVAL;
		return NULL;
	}

	uint32_t shaderIndices[dsVectorShaderType_Count];
	for (uint32_t i = 0; i < (uint32_t)dsVectorShaderType_Count; ++i)
	{
		if (shaderNames[i])
			shaderIndices[i] = DS_MATERIAL_UNKNOWN;
		else
			shaderIndices[i] = shaderModule->shaderIndices[i];
	}

	uint32_t shaderCount = dsShaderModule_shaderCount(shaderModule->shaderModule);
	for (uint32_t i = 0; i < (uint32_t)dsVectorShaderType_Count; ++i)
	{
		if (!shaderNames[i])
			continue;

		for (uint32_t j = 0; j < shaderCount; ++j)
		{
			const char* name = dsShaderModule_shaderName(shaderModule->shaderModule, j);
			if (strcmp(name, shaderNames[i]) == 0)
			{
				shaderIndices[i] = j;
				break;
			}
		}
	}

	return dsVectorShaders_createImpl(resourceManager, allocator, shaderModule, shaderIndices,
		shaderNames);
}

bool dsVectorShaders_destroy(dsVectorShaders* shaders)
{
	if (!shaders)
		return true;

	for (uint32_t i = 0; i < (uint32_t)dsVectorShaderType_Count; ++i)
	{
		if (!dsShader_destroy(shaders->shaders[i]))
		{
			DS_ASSERT(i == 0);
			return false;
		}
	}

	if (shaders->allocator)
		DS_VERIFY(dsAllocator_free(shaders->allocator, shaders));
	return true;
}
