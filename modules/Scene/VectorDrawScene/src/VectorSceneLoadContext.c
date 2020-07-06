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

#include <DeepSea/VectorDrawScene/VectorSceneLoadContext.h>

#include "SceneTextLoad.h"
#include "SceneTextNodeLoad.h"
#include "SceneVectorImageLoad.h"
#include "SceneVectorImageNodeLoad.h"
#include "SceneVectorItemListLoad.h"
#include "VectorSceneResourcesLoad.h"
#include "VectorSceneMaterialSetLoad.h"
#include "VectorSceneShadersLoad.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Scene/SceneLoadContext.h>
#include <DeepSea/Text/TextSubstitutionData.h>
#include <DeepSea/VectorDraw/VectorImage.h>
#include <DeepSea/VectorDraw/VectorMaterialSet.h>
#include <DeepSea/VectorDraw/VectorResources.h>
#include <DeepSea/VectorDraw/VectorScratchData.h>
#include <DeepSea/VectorDrawScene/SceneText.h>
#include <DeepSea/VectorDrawScene/SceneTextNode.h>
#include <DeepSea/VectorDrawScene/SceneVectorImage.h>
#include <DeepSea/VectorDrawScene/SceneVectorImageNode.h>
#include <DeepSea/VectorDrawScene/SceneVectorItemList.h>
#include <DeepSea/VectorDrawScene/VectorSceneMaterialSet.h>
#include <DeepSea/VectorDrawScene/VectorSceneResources.h>
#include <DeepSea/VectorDrawScene/VectorSceneShaders.h>

#include <string.h>

static void VectorResourcesUserData_destroy(void* userData)
{
	if (!userData)
		return;

	VectorResourcesUserData* vectorResourcesUserData = (VectorResourcesUserData*)userData;
	if (vectorResourcesUserData->allocator)
		DS_VERIFY(dsAllocator_free(vectorResourcesUserData->allocator, userData));
}

static void SceneTextUserData_destroy(void* userData)
{
	if (!userData)
		return;

	SceneTextUserData* sceneTextUserData = (SceneTextUserData*)userData;
	dsTextSubstitutionData_destroy(sceneTextUserData->substitutionData);
	if (sceneTextUserData->allocator)
		DS_VERIFY(dsAllocator_free(sceneTextUserData->allocator, userData));
}

static void SceneVectorImageUserData_destroy(void* userData)
{
	if (!userData)
		return;

	SceneVectorImageUserData* vectorImageUserData = (SceneVectorImageUserData*)userData;
	dsVectorScratchData_destroy(vectorImageUserData->scratchData);
	if (vectorImageUserData->allocator)
		DS_VERIFY(dsAllocator_free(vectorImageUserData->allocator, userData));
}

static void SceneVectorItemListUserData_destroy(void* userData)
{
	if (!userData)
		return;

	SceneVectorItemListUserData* vectorItemListUserData = (SceneVectorItemListUserData*)userData;
	if (vectorItemListUserData->allocator)
		DS_VERIFY(dsAllocator_free(vectorItemListUserData->allocator, vectorItemListUserData));
}

static bool destroySceneText(void* text)
{
	dsSceneText_destroy((dsSceneText*)text);
	return true;
}

bool dsVectorSceneLoadConext_registerTypes(dsSceneLoadContext* loadContext, dsAllocator* allocator,
	dsCommandBuffer* commandBuffer, const dsTextQuality* qualityRemap,
	const dsTextSubstitutionTable* substitutionTable,
	const dsSceneTextRenderBufferInfo* textRenderInfo, float pixelSize)
{
	if (!loadContext || (!allocator && (commandBuffer || qualityRemap || substitutionTable)) ||
		pixelSize <= 0.0f)
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

		if (!dsSceneLoadContext_registerCustomSceneResourceType(loadContext,
				dsVectorSceneResources_typeName, dsVectorSceneResources_type(),
				&dsVectorSceneResources_load,
				(dsDestroyCustomSceneResourceFunction)&dsVectorResources_destroy, userData,
				&VectorResourcesUserData_destroy))
		{
			VectorResourcesUserData_destroy(userData);
			return false;
		}
	}

	if (!dsSceneLoadContext_registerCustomSceneResourceType(loadContext,
			dsVectorSceneMaterialSet_typeName, dsVectorSceneMaterialSet_type(),
			&dsVectorSceneMaterialSet_load,
			(dsDestroyCustomSceneResourceFunction)&dsVectorMaterialSet_destroy, NULL, NULL))
	{
		return false;
	}

	if (!dsSceneLoadContext_registerCustomSceneResourceType(loadContext,
			dsVectorSceneShaders_typeName, dsVectorSceneShaders_type(), &dsVectorSceneShaders_load,
			dsVectorSceneShaders_destroy, NULL, NULL))
	{
		return false;
	}

	{
		SceneTextUserData* userData = NULL;
		if (substitutionTable)
		{
			userData = DS_ALLOCATE_OBJECT(allocator, SceneTextUserData);
			if (!userData)
				return false;

			dsTextSubstitutionData* substitutionData = dsTextSubstitutionData_create(allocator);
			if (!substitutionData)
			{
				if (allocator->freeFunc)
					DS_VERIFY(dsAllocator_free(allocator, userData));
				return false;
			}

			userData->allocator = dsAllocator_keepPointer(allocator);
			userData->substitutionTable = substitutionTable;
			userData->substitutionData = substitutionData;
			userData->pixelScale = 1.0f/pixelSize;
		}

		if (!dsSceneLoadContext_registerCustomSceneResourceType(loadContext,
				dsSceneText_typeName, dsSceneText_type(), &dsSceneText_load, destroySceneText,
				userData, &SceneTextUserData_destroy))
		{
			SceneTextUserData_destroy(userData);
			return false;
		}
	}

	if (commandBuffer)
	{
		dsVectorScratchData* scratchData = dsVectorScratchData_create(allocator);

		SceneVectorImageUserData* userData =
			DS_ALLOCATE_OBJECT(allocator, SceneVectorImageUserData);
		if (!userData)
		{
			dsVectorScratchData_destroy(scratchData);
			return false;
		}

		userData->allocator = dsAllocator_keepPointer(allocator);
		userData->commandBuffer = commandBuffer;
		userData->scratchData = scratchData;
		userData->pixelSize = pixelSize;

		if (!dsSceneLoadContext_registerCustomSceneResourceType(loadContext,
				dsSceneVectorImage_typeName, dsSceneVectorImage_type(), &dsSceneVectorImage_load,
				(dsDestroyCustomSceneResourceFunction)&dsVectorImage_destroy,
				userData, &SceneVectorImageUserData_destroy))
		{
			SceneVectorImageUserData_destroy(userData);
			return false;
		}
	}

	{
		SceneVectorItemListUserData* userData = NULL;
		if (textRenderInfo)
		{
			userData = DS_ALLOCATE_OBJECT(allocator, SceneVectorItemListUserData);
			if (!userData)
				return false;

			userData->allocator = dsAllocator_keepPointer(allocator);
			userData->textRenderInfo = *textRenderInfo;
		}

		if (!dsSceneLoadContext_registerItemListType(loadContext, dsSceneVectorItemList_typeName,
				&dsSceneVectorItemList_load, userData, &SceneVectorItemListUserData_destroy))
		{
			SceneVectorItemListUserData_destroy(userData);
			return false;
		}
	}

	if (!dsSceneLoadContext_registerNodeType(loadContext, dsSceneTextNode_typeName,
			&dsSceneTextNode_load, NULL, NULL))
	{
		return false;
	}

	if (!dsSceneLoadContext_registerNodeType(loadContext, dsSceneVectorImageNode_typeName,
			&dsSceneVectorImageNode_load, NULL, NULL))
	{
		return false;
	}

	return true;
}
