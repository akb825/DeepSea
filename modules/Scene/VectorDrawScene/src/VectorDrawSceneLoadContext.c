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

#include <DeepSea/VectorDrawScene/VectorDrawSceneLoadContext.h>

#include "VectorDrawSceneResourcesLoad.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Scene/SceneLoadContext.h>
#include <DeepSea/VectorDraw/VectorResources.h>

#include <string.h>

void VectorResourcesUserData_destroy(void* userData)
{
	if (!userData)
		return;

	VectorResourcesUserData* vectorResourcesUserData = (VectorResourcesUserData*)userData;
	if (vectorResourcesUserData->allocator)
		DS_VERIFY(dsAllocator_free(vectorResourcesUserData->allocator, userData));
}

static dsCustomSceneResourceType resourceType;
const dsCustomSceneResourceType* dsVectorDrawSceneLoadContext_getVectorResourcesType(void)
{
	return &resourceType;
}

bool dsVectorDrawSceneLoadConext_registerTypes(dsSceneLoadContext* loadContext,
	dsAllocator* allocator, const dsTextQuality* qualityRemap)
{
	if (!loadContext || (qualityRemap && !allocator))
	{
		errno = EINVAL;
		return false;
	}

	{
		VectorResourcesUserData* userData = NULL;
		if (qualityRemap)
		{
			userData = DS_ALLOCATE_OBJECT(allocator, VectorResourcesUserData);
			if (!userData)
				return false;

			userData->allocator = dsAllocator_keepPointer(allocator);
			memcpy(userData, qualityRemap,
				sizeof(dsTextQuality)*DS_TEXT_QUALITY_REMAP_SIZE);
		}

		if (!dsSceneLoadContext_registerCustomSceneResourceType(loadContext, "VectorResources",
				&resourceType, &dsVectorDrawSceneResources_load,
				(dsDestroyCustomSceneResourceFunction)&dsVectorResources_destroy, userData,
				&VectorResourcesUserData_destroy))
		{
			VectorResourcesUserData_destroy(userData);
			return false;
		}
	}
	return true;
}
