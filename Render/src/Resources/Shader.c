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

#include <DeepSea/Render/Resources/Shader.h>

#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Render/Resources/ResourceManager.h>
#include <DeepSea/Render/Resources/ShaderModule.h>
#include <DeepSea/Render/Types.h>
#include <string.h>

extern const char* dsResourceManager_noContextError;

dsShader* dsShader_createName(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsShaderModule* shaderModule, const char* name, const dsMaterialDesc* materialDesc)
{
	if (!resourceManager || !resourceManager->createShaderFunc ||
		!resourceManager->destroyShaderFunc || !shaderModule || !name || !materialDesc)
	{
		errno = EINVAL;
		return NULL;
	}

	uint32_t index = 0, shaderCount = dsShaderModule_shaderCount(shaderModule);
	for (; index < shaderCount; ++index)
	{
		if (strcmp(name, dsShaderModule_shaderName(shaderModule, index)) == 0)
			break;
	}

	if (index == shaderCount)
	{
		errno = EINVAL;
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Shader %s not found in shader module.", name);
		return NULL;
	}

	return dsShader_createIndex(resourceManager, allocator, shaderModule, index, materialDesc);
}

dsShader* dsShader_createIndex(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsShaderModule* shaderModule, uint32_t index, const dsMaterialDesc* materialDesc)
{
	DS_PROFILE_FUNC_START();

	if (!resourceManager || !resourceManager->createShaderFunc ||
		!resourceManager->destroyShaderFunc || !shaderModule || !materialDesc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!allocator)
		allocator = resourceManager->allocator;

	if (index >= dsShaderModule_shaderCount(shaderModule))
	{
		errno = EINDEX;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, dsResourceManager_noContextError);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsShader* shader = resourceManager->createShaderFunc(resourceManager, allocator, shaderModule,
		index, materialDesc);
	if (shader)
		DS_ATOMIC_FETCH_ADD32(&resourceManager->shaderCount, 1);
	DS_PROFILE_FUNC_RETURN(shader);
}

bool dsShader_destroy(dsShader* shader)
{
	DS_PROFILE_FUNC_START();

	if (!shader || !shader->resourceManager || !shader->resourceManager->destroyShaderFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = shader->resourceManager;
	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, dsResourceManager_noContextError);
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = resourceManager->destroyShaderFunc(resourceManager, shader);
	if (success)
		DS_ATOMIC_FETCH_ADD32(&resourceManager->shaderCount, -1);
	DS_PROFILE_FUNC_RETURN(success);
}
