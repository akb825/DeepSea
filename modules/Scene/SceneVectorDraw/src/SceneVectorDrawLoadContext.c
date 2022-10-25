/*
 * Copyright 2020-2022 Aaron Barany
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

#include <DeepSea/SceneVectorDraw/SceneVectorDrawLoadContext.h>

#include "SceneTextLoad.h"
#include "SceneTextNodeLoad.h"
#include "SceneVectorDrawPrepareLoad.h"
#include "SceneVectorImageLoad.h"
#include "SceneVectorImageNodeLoad.h"
#include "SceneVectorItemListLoad.h"
#include "SceneVectorMaterialSetLoad.h"
#include "SceneVectorResourcesLoad.h"
#include "SceneVectorShadersLoad.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>

#include <DeepSea/Scene/SceneLoadContext.h>

#include <DeepSea/SceneVectorDraw/SceneText.h>
#include <DeepSea/SceneVectorDraw/SceneTextNode.h>
#include <DeepSea/SceneVectorDraw/SceneVectorDrawPrepare.h>
#include <DeepSea/SceneVectorDraw/SceneVectorImage.h>
#include <DeepSea/SceneVectorDraw/SceneVectorImageNode.h>
#include <DeepSea/SceneVectorDraw/SceneVectorItemList.h>
#include <DeepSea/SceneVectorDraw/SceneVectorMaterialSet.h>
#include <DeepSea/SceneVectorDraw/SceneVectorResources.h>
#include <DeepSea/SceneVectorDraw/SceneVectorShaders.h>

#include <DeepSea/Text/TextSubstitutionData.h>

#include <DeepSea/VectorDraw/VectorImage.h>
#include <DeepSea/VectorDraw/VectorMaterialSet.h>
#include <DeepSea/VectorDraw/VectorResources.h>
#include <DeepSea/VectorDraw/VectorScratchData.h>

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

static void SceneTextNodeUserData_destroy(void* userData)
{
	if (!userData)
		return;

	SceneTextNodeUserData* vectorItemListUserData = (SceneTextNodeUserData*)userData;
	if (vectorItemListUserData->allocator)
		DS_VERIFY(dsAllocator_free(vectorItemListUserData->allocator, vectorItemListUserData));
}

static bool destroySceneText(void* text)
{
	dsSceneText_destroy((dsSceneText*)text);
	return true;
}

bool dsSceneVectorDrawLoadConext_registerTypes(dsSceneLoadContext* loadContext, dsAllocator* allocator,
	dsCommandBuffer* commandBuffer, const dsTextQuality* qualityRemap,
	const dsTextSubstitutionTable* substitutionTable,
	const dsSceneTextRenderBufferInfo* textRenderInfo, float pixelSize)
{
	if (!loadContext ||
		(!allocator && (commandBuffer || qualityRemap || substitutionTable || textRenderInfo)) ||
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

		// One additional resource for registering the material description.
		if (!dsSceneLoadContext_registerCustomResourceType(loadContext,
				dsSceneVectorResources_typeName, dsSceneVectorResources_type(),
				&dsVectorSceneResources_load,
				(dsDestroyCustomSceneResourceFunction)&dsVectorResources_destroy, userData,
				&VectorResourcesUserData_destroy, 1))
		{
			VectorResourcesUserData_destroy(userData);
			return false;
		}
	}

	if (!dsSceneLoadContext_registerCustomResourceType(loadContext,
			dsSceneVectorMaterialSet_typeName, dsSceneVectorMaterialSet_type(),
			&dsVectorSceneMaterialSet_load,
			(dsDestroyCustomSceneResourceFunction)&dsVectorMaterialSet_destroy, NULL, NULL, 0))
	{
		return false;
	}

	if (!dsSceneLoadContext_registerCustomResourceType(loadContext,
			dsSceneVectorShaders_typeName, dsSceneVectorShaders_type(), &dsSceneVectorShaders_load,
			dsSceneVectorShaders_destroy, NULL, NULL, 0))
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

		if (!dsSceneLoadContext_registerCustomResourceType(loadContext,
				dsSceneText_typeName, dsSceneText_type(), &dsSceneText_load, destroySceneText,
				userData, &SceneTextUserData_destroy, 0))
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

		if (!dsSceneLoadContext_registerCustomResourceType(loadContext,
				dsSceneVectorImage_typeName, dsSceneVectorImage_type(), &dsSceneVectorImage_load,
				(dsDestroyCustomSceneResourceFunction)&dsVectorImage_destroy,
				userData, &SceneVectorImageUserData_destroy, 0))
		{
			SceneVectorImageUserData_destroy(userData);
			return false;
		}
	}

	if (!dsSceneLoadContext_registerItemListType(loadContext, dsSceneVectorItemList_typeName,
			&dsSceneVectorItemList_load, NULL, NULL))
	{
		return false;
	}

	if (!dsSceneLoadContext_registerItemListType(loadContext, dsSceneVectorDrawPrepare_typeName,
			&dsSceneVectorDrawPrepare_load, NULL, NULL))
	{
		return false;
	}

	if (textRenderInfo && !dsSceneVectorDrawLoadContext_registerCustomTextNodeType(loadContext,
			allocator, dsSceneTextNode_typeName, textRenderInfo))
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

bool dsSceneVectorDrawLoadContext_registerCustomTextNodeType(
	dsSceneLoadContext* loadContext, dsAllocator* allocator, const char* name,
	const dsSceneTextRenderBufferInfo* textRenderInfo)
{
	if (!loadContext || !allocator || !name || !textRenderInfo)
	{
		errno = EINVAL;
		return false;
	}

	SceneTextNodeUserData* userData = DS_ALLOCATE_OBJECT(allocator, SceneTextNodeUserData);
	if (!userData)
		return false;

	userData->allocator = dsAllocator_keepPointer(allocator);
	userData->textRenderInfo = *textRenderInfo;

	if (!dsSceneLoadContext_registerNodeType(loadContext, name, &dsSceneTextNode_load, userData,
			&SceneTextNodeUserData_destroy))
	{
		return false;
	}

	return true;
}
