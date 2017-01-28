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

#include <DeepSea/Render/Resources/MaterialDesc.h>

#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Render/Resources/ResourceManager.h>
#include <DeepSea/Render/Types.h>

dsMaterialDesc* dsMaterialDesc_create(dsResourceManager* resourceManager,
	dsAllocator* allocator, const dsMaterialElement* elements, uint32_t elementCount)
{
	DS_PROFILE_FUNC_START();

	if (!resourceManager || (!allocator && !resourceManager->allocator) ||
		!resourceManager->createMaterialDescFunc || !resourceManager->destroyMaterialDescFunc ||
		(!elements && elementCount > 0))
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!allocator)
		allocator = resourceManager->allocator;

	for (uint32_t i = 0; i < elementCount; ++i)
	{
		if (!elements[i].name || elements[i].count == 0)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Invalid material element.");
			DS_PROFILE_FUNC_RETURN(NULL);
		}

		if (elements[i].type == dsMaterialType_UniformBlock &&
			!(resourceManager->supportedBuffers & dsGfxBufferUsage_UniformBlock))
		{
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Target doesn't support uniform blocks.");
			DS_PROFILE_FUNC_RETURN(NULL);
		}

		if (elements[i].type == dsMaterialType_UniformBuffer &&
			!(resourceManager->supportedBuffers & dsGfxBufferUsage_UniformBuffer))
		{
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Target doesn't support uniform buffers.");
			DS_PROFILE_FUNC_RETURN(NULL);
		}
	}

	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Resources can only be manipulated from the main thread or "
			"threads that have created a resource context.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsMaterialDesc* materialDesc = resourceManager->createMaterialDescFunc(resourceManager,
		allocator, elements, elementCount);
	if (materialDesc)
		DS_ATOMIC_FETCH_ADD32(&resourceManager->materialDescCount, 1);
	DS_PROFILE_FUNC_RETURN(materialDesc);
}

bool dsMaterialDesc_destroy(dsMaterialDesc* materialDesc)
{
	DS_PROFILE_FUNC_START();

	if (!materialDesc || !materialDesc->resourceManager ||
		!materialDesc->resourceManager->destroyMaterialDescFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = materialDesc->resourceManager;
	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Resources can only be manipulated from the main thread or "
			"threads that have created a resource context.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = resourceManager->destroyMaterialDescFunc(resourceManager, materialDesc);
	if (success)
		DS_ATOMIC_FETCH_ADD32(&resourceManager->materialDescCount, -1);
	DS_PROFILE_FUNC_RETURN(success);
}
