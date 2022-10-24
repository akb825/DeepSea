/*
 * Copyright 2019-2022 Aaron Barany
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

#include "LightData.h"
#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Vector3.h>
#include <DeepSea/Render/Resources/ShaderVariableGroup.h>
#include <DeepSea/Render/Resources/ShaderVariableGroupDesc.h>
#include <DeepSea/Render/Resources/SharedMaterialValues.h>
#include <DeepSea/Scene/SceneLoadContext.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif

#include "LightData_generated.h"
#include <DeepSea/Scene/Flatbuffers/SceneCommon_generated.h>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#endif

#include <string.h>

typedef struct dsLightData
{
	dsSceneItemList globalData;
	dsShaderVariableGroup* variableGroup;
	dsVector3f direction;
	uint32_t variableGroupNameID;
} dsLightData;

static int type;

static void dsLightData_commit(dsSceneItemList* itemList, const dsView* view,
	dsCommandBuffer* commandBuffer)
{
	dsLightData* lightData = (dsLightData*)itemList;
	dsVector4f direction =
		{{lightData->direction.x, lightData->direction.y, lightData->direction.z, 0.0f}};
	dsVector4f viewDirection;
	dsMatrix44_transform(viewDirection, view->viewMatrix, direction);
	DS_VERIFY(dsShaderVariableGroup_setElementData(lightData->variableGroup, 0, &viewDirection,
		dsMaterialType_Vec3, 0, 1));
	DS_CHECK("TestScene", dsShaderVariableGroup_commit(lightData->variableGroup, commandBuffer));
	DS_VERIFY(dsSharedMaterialValues_setVariableGroupID(view->globalValues,
		lightData->variableGroupNameID, lightData->variableGroup));
}

static void dsLightData_destroy(dsSceneItemList* itemList)
{
	dsLightData* lightData = (dsLightData*)itemList;
	DS_VERIFY(dsShaderVariableGroup_destroy(lightData->variableGroup));

	if (itemList->allocator)
		DS_VERIFY(dsAllocator_free(itemList->allocator, itemList));
}

dsSceneItemList* dsLightData_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator*, void*,
	const char* name, const uint8_t* data, size_t dataSize)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!TestScene::VerifyLightDataBuffer(verifier))
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Invalid instance transform data flatbuffer format.");
		return nullptr;
	}

	auto fbLightData = TestScene::GetLightData(data);
	const char* groupDescName = fbLightData->variableGroupDesc()->c_str();
	dsVector3f direction = *reinterpret_cast<const dsVector3f*>(fbLightData->direction());
	dsVector3f color = *reinterpret_cast<const dsVector3f*>(fbLightData->color());
	dsVector3f ambient = *reinterpret_cast<const dsVector3f*>(fbLightData->ambient());

	dsShaderVariableGroupDesc* groupDesc;
	dsSceneResourceType resourceType;
	if (!dsSceneLoadScratchData_findResource(&resourceType, reinterpret_cast<void**>(&groupDesc),
			scratchData, groupDescName) ||
		resourceType != dsSceneResourceType_ShaderVariableGroupDesc)
	{
		// NOTE: ENOTFOUND not set when the type doesn't match, so set it manually.
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG,
			"Couldn't find light data shader variable group description '%s'.", groupDescName);
		return nullptr;
	}

	dsRenderer* renderer = dsSceneLoadContext_getRenderer(loadContext);
	dsSceneItemList* lightData =
		dsLightData_create(allocator, name, renderer->resourceManager, groupDesc);
	if (!lightData)
		return nullptr;

	dsLightData_setDirection(lightData, &direction);
	dsLightData_setColor(lightData, &color);
	dsLightData_setAmbientColor(lightData, &ambient);
	return lightData;
}

dsSceneItemList* dsLightData_create(dsAllocator* allocator, const char* name,
	dsResourceManager* resourceManager, const dsShaderVariableGroupDesc* lightDesc)
{
	DS_ASSERT(allocator);
	DS_ASSERT(resourceManager);
	DS_ASSERT(lightDesc);

	size_t nameLen = strlen(name) + 1;
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsLightData)) + DS_ALIGNED_SIZE(nameLen) +
		dsShaderVariableGroup_fullAllocSize(resourceManager, lightDesc);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return nullptr;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsLightData* lightData = DS_ALLOCATE_OBJECT(&bufferAlloc, dsLightData);
	DS_VERIFY(lightData);
	dsSceneItemList* itemList = (dsSceneItemList*)lightData;

	itemList->allocator = dsAllocator_keepPointer(allocator);
	itemList->type = &type;
	itemList->name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
	DS_ASSERT(itemList->name);
	memcpy((void*)itemList->name, name, nameLen);
	itemList->nameID = dsHashString(name);
	itemList->globalValueCount = 1;
	itemList->needsCommandBuffer = true;
	itemList->addNodeFunc = NULL;
	itemList->updateNodeFunc = NULL;
	itemList->removeNodeFunc = NULL;
	itemList->updateFunc = NULL;
	itemList->commitFunc = &dsLightData_commit;
	itemList->destroyFunc = &dsLightData_destroy;

	lightData->variableGroup = dsShaderVariableGroup_create(resourceManager,
		(dsAllocator*)&bufferAlloc, allocator, lightDesc);
	if (!lightData->variableGroup)
	{
		if (allocator->freeFunc)
			DS_VERIFY(dsAllocator_free(allocator, buffer));
		return nullptr;
	}

	const char* variableGroupName = "LightData";
	lightData->variableGroupNameID = dsHashString(variableGroupName);

	return itemList;
}

void dsLightData_setDirection(dsSceneItemList* itemList, const dsVector3f* direction)
{
	DS_VERIFY(itemList);
	DS_VERIFY(direction);
	dsLightData* lightData = (dsLightData*)itemList;
	dsVector3f_normalize(&lightData->direction, direction);
}

void dsLightData_setColor(dsSceneItemList* itemList, const dsVector3f* color)
{
	DS_VERIFY(itemList);
	DS_VERIFY(color);
	dsLightData* lightData = (dsLightData*)itemList;
	DS_VERIFY(dsShaderVariableGroup_setElementData(lightData->variableGroup, 1, color,
		dsMaterialType_Vec3, 0, 1));
}

void dsLightData_setAmbientColor(dsSceneItemList* itemList, const dsVector3f* color)
{
	DS_VERIFY(itemList);
	DS_VERIFY(color);
	dsLightData* lightData = (dsLightData*)itemList;
	DS_VERIFY(dsShaderVariableGroup_setElementData(lightData->variableGroup, 2, color,
		dsMaterialType_Vec3, 0, 1));
}
