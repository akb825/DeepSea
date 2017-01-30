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

#include <DeepSea/Render/Resources/ShaderVariableGroupDesc.h>

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Render/Resources/ResourceManager.h>
#include <DeepSea/Render/Types.h>
#include <string.h>

extern const char* dsResourceManager_noContextError;

dsShaderVariableGroupDesc* dsShaderVariableGroupDesc_create(dsResourceManager* resourceManager,
	dsAllocator* allocator, const dsMaterialElement* elements, uint32_t elementCount)
{
	DS_PROFILE_FUNC_START();

	if (!resourceManager || (!allocator && !resourceManager->allocator) ||
		!resourceManager->createShaderVariableGroupDescFunc ||
		!resourceManager->destroyShaderVariableGroupDescFunc || !elements || elementCount == 0)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!allocator)
		allocator = resourceManager->allocator;

	for (uint32_t i = 0; i < elementCount; ++i)
	{
		if (!elements[i].name || (int)elements[i].type < 0 ||
			elements[i].type >= dsMaterialType_Count)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Invalid material element.");
			DS_PROFILE_FUNC_RETURN(NULL);
		}

		if (elements[i].type >= dsMaterialType_Texture)
		{
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG,
				"Shader variable groups cannot contain texture, image, block, or buffer types.");
			DS_PROFILE_FUNC_RETURN(NULL);
		}
	}

	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, dsResourceManager_noContextError);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsShaderVariableGroupDesc* groupDesc = resourceManager->createShaderVariableGroupDescFunc(
		resourceManager, allocator, elements, elementCount);
	if (groupDesc)
	{
		DS_ATOMIC_FETCH_ADD32(&resourceManager->shaderVariableGroupDescCount, 1);
		// Sanity check
		DS_ASSERT(((resourceManager->supportedBuffers & dsGfxBufferUsage_UniformBlock) &&
			groupDesc->positions) ||
			(!(resourceManager->supportedBuffers & dsGfxBufferUsage_UniformBlock) &&
			!groupDesc->positions));
	}
	DS_PROFILE_FUNC_RETURN(groupDesc);
}

uint32_t dsShaderVariableGroupDesc_findElement(const dsShaderVariableGroupDesc* groupDesc,
	const char* name)
{
	if (!groupDesc || !name)
		return DS_UNKNOWN;

	for (uint32_t i = 0; i < groupDesc->elementCount; ++i)
	{
		if (strcmp(groupDesc->elements[i].name, name) == 0)
			return i;
	}

	return DS_UNKNOWN;
}

bool dsShaderVariableGroupDesc_destroy(dsShaderVariableGroupDesc* groupDesc)
{
	DS_PROFILE_FUNC_START();

	if (!groupDesc || !groupDesc->resourceManager ||
		!groupDesc->resourceManager->destroyShaderVariableGroupDescFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = groupDesc->resourceManager;
	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, dsResourceManager_noContextError);
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = resourceManager->destroyShaderVariableGroupDescFunc(resourceManager,
		groupDesc);
	if (success)
		DS_ATOMIC_FETCH_ADD32(&resourceManager->shaderVariableGroupDescCount, -1);
	DS_PROFILE_FUNC_RETURN(success);
}

