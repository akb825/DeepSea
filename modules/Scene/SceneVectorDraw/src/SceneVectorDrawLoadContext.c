/*
 * Copyright 2020-2026 Aaron Barany
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

#include "InstanceDiscardBoundsDataLoad.h"
#include "SceneDiscardBoundsNodeLoad.h"
#include "SceneTextLoad.h"
#include "SceneTextNodeLoad.h"
#include "SceneVectorDrawScratchData.h"
#include "SceneVectorDrawTypes.h"
#include "SceneVectorDrawPrepareLoad.h"
#include "SceneVectorImageLoad.h"
#include "SceneVectorImageNodeLoad.h"
#include "SceneVectorItemListLoad.h"
#include "SceneVectorMaterialSetLoad.h"
#include "SceneVectorResourcesLoad.h"
#include "SceneVectorShadersLoad.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Thread/ThreadObjectStorage.h>
#include <DeepSea/Core/Assert.h>

#include <DeepSea/Scene/SceneLoadContext.h>

#include <DeepSea/SceneVectorDraw/InstanceDiscardBoundsData.h>
#include <DeepSea/SceneVectorDraw/SceneDiscardBoundsNode.h>
#include <DeepSea/SceneVectorDraw/SceneText.h>
#include <DeepSea/SceneVectorDraw/SceneTextNode.h>
#include <DeepSea/SceneVectorDraw/SceneVectorDrawPrepare.h>
#include <DeepSea/SceneVectorDraw/SceneVectorImage.h>
#include <DeepSea/SceneVectorDraw/SceneVectorImageNode.h>
#include <DeepSea/SceneVectorDraw/SceneVectorItemList.h>
#include <DeepSea/SceneVectorDraw/SceneVectorMaterialSet.h>
#include <DeepSea/SceneVectorDraw/SceneVectorResources.h>
#include <DeepSea/SceneVectorDraw/SceneVectorShaders.h>

#include <DeepSea/VectorDraw/VectorImage.h>
#include <DeepSea/VectorDraw/VectorMaterialSet.h>
#include <DeepSea/VectorDraw/VectorResources.h>

#include <string.h>

static void dsSceneVectorDrawLoadContext_destroy(void* userData)
{
	dsSceneVectorDrawLoadContext* loadContext = (dsSceneVectorDrawLoadContext*)userData;
	dsThreadObjectStorage_destroy(loadContext->scratchData);
	DS_VERIFY(dsAllocator_free(loadContext->allocator, loadContext));
}

static void dsSceneTextNodeUserData_destroy(void* userData)
{
	if (!userData)
		return;

	dsSceneTextNodeUserData* vectorItemListUserData = (dsSceneTextNodeUserData*)userData;
	if (vectorItemListUserData->allocator)
		DS_VERIFY(dsAllocator_free(vectorItemListUserData->allocator, vectorItemListUserData));
}

static bool destroySceneText(void* text)
{
	dsSceneText_destroy((dsSceneText*)text);
	return true;
}

dsSceneVectorDrawLoadContext* dsSceneVectorDrawLoadConext_registerTypes(
	dsSceneLoadContext* loadContext, dsAllocator* allocator, const dsTextQuality* qualityRemap,
	const dsTextSubstitutionTable* substitutionTable,
	const dsSceneTextRenderBufferInfo* textRenderInfo, float pixelSize)
{
	if (!loadContext || pixelSize <= 0.0f)
	{
		errno = EINVAL;
		return false;
	}

	if (!allocator)
		allocator = dsSceneLoadContext_getAllocator(loadContext);
	if (!allocator || !allocator->freeFunc)
	{
		DS_LOG_ERROR(DS_SCENE_VECTOR_DRAW_LOG_TAG,
			"Scene vector draw load context allocator must support freeing memory.");
		errno = EINVAL;
		return NULL;
	}

	dsSceneVectorDrawLoadContext* vectorLoadContext = DS_ALLOCATE_OBJECT(
		allocator, dsSceneVectorDrawLoadContext);
	if (!vectorLoadContext)
		return NULL;

	vectorLoadContext->allocator = allocator;
	vectorLoadContext->scratchData = dsThreadObjectStorage_create(
		allocator, &dsSceneVectorDrawScratchData_destroy);
	if (!vectorLoadContext->scratchData)
	{
		dsSceneVectorDrawLoadContext_destroy(vectorLoadContext);
		return NULL;
	}

	if (qualityRemap)
	{
		memcpy(vectorLoadContext->textQualityRemap, qualityRemap,
			sizeof(dsTextQuality)*DS_TEXT_QUALITY_REMAP_SIZE);
	}
	else
	{
		for (int i = 0; i < DS_TEXT_QUALITY_REMAP_SIZE; ++i)
			vectorLoadContext->textQualityRemap[i] = (dsTextQuality)i;
	}

	vectorLoadContext->substitutionTable = substitutionTable;
	vectorLoadContext->pixelSize = pixelSize;

	// This is responsible for destroying the vector load context.
	if (!dsSceneLoadContext_registerCustomResourceType(loadContext,
			dsSceneVectorResources_typeName, dsSceneVectorResources_type(),
			&dsVectorSceneResources_load,
			(dsDestroyCustomSceneResourceFunction)&dsVectorResources_destroy, vectorLoadContext,
			&dsSceneVectorDrawLoadContext_destroy, 0))
	{
		return NULL;
	}

	if (!dsSceneLoadContext_registerCustomResourceType(loadContext,
			dsSceneVectorMaterialSet_typeName, dsSceneVectorMaterialSet_type(),
			&dsVectorSceneMaterialSet_load,
			(dsDestroyCustomSceneResourceFunction)&dsVectorMaterialSet_destroy, NULL, NULL, 0))
	{
		return NULL;
	}

	// One additional resource for registering the material description.
	if (!dsSceneLoadContext_registerCustomResourceType(loadContext,
			dsSceneVectorShaders_typeName, dsSceneVectorShaders_type(), &dsSceneVectorShaders_load,
			dsSceneVectorShaders_destroy, NULL, NULL, 1))
	{
		return NULL;
	}

	if (!dsSceneLoadContext_registerCustomResourceType(loadContext,
			dsSceneText_typeName, dsSceneText_type(), &dsSceneText_load, destroySceneText,
			vectorLoadContext, NULL, 0))
	{
		return NULL;
	}

	if (!dsSceneLoadContext_registerCustomResourceType(loadContext,
			dsSceneVectorImage_typeName, dsSceneVectorImage_type(), &dsSceneVectorImage_load,
			(dsDestroyCustomSceneResourceFunction)&dsSceneVectorImage_destroy, vectorLoadContext,
			NULL, 0))
	{
		return NULL;
	}

	if (!dsSceneLoadContext_registerItemListType(loadContext, dsSceneVectorItemList_typeName,
			&dsSceneVectorItemList_load, NULL, NULL))
	{
		return NULL;
	}

	if (!dsSceneLoadContext_registerItemListType(loadContext, dsSceneVectorDrawPrepare_typeName,
			&dsSceneVectorDrawPrepare_load, NULL, NULL))
	{
		return NULL;
	}

	if (!dsSceneLoadContext_registerNodeType(loadContext, dsSceneDiscardBoundsNode_typeName,
			&dsSceneDiscardBoundsNode_load, NULL, NULL))
	{
		return NULL;
	}

	if (textRenderInfo && !dsSceneVectorDrawLoadContext_registerCustomTextNodeType(
			loadContext, allocator, dsSceneTextNode_typeName, textRenderInfo))
	{
		return NULL;
	}

	if (!dsSceneLoadContext_registerNodeType(
			loadContext, dsSceneVectorImageNode_typeName, &dsSceneVectorImageNode_load, NULL, NULL))
	{
		return NULL;
	}

	if (!dsSceneLoadContext_registerInstanceDataType(loadContext,
			dsInstanceDiscardBoundsData_typeName, &dsInstanceDiscardBoundsData_load, NULL, NULL))
	{
		return NULL;
	}

	return vectorLoadContext;
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

	dsSceneTextNodeUserData* userData = DS_ALLOCATE_OBJECT(allocator, dsSceneTextNodeUserData);
	if (!userData)
		return false;

	userData->allocator = dsAllocator_keepPointer(allocator);
	userData->textRenderInfo = *textRenderInfo;

	return dsSceneLoadContext_registerNodeType(loadContext, name, &dsSceneTextNode_load, userData,
		&dsSceneTextNodeUserData_destroy);
}
