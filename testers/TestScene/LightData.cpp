/*
 * Copyright 2019-2021 Aaron Barany
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
#include "LightData_generated.h"
#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Vector3.h>
#include <DeepSea/Render/Resources/ShaderVariableGroup.h>
#include <DeepSea/Render/Resources/ShaderVariableGroupDesc.h>
#include <DeepSea/Render/Resources/SharedMaterialValues.h>
#include <DeepSea/Scene/Flatbuffers/SceneCommon_generated.h>
#include <DeepSea/Scene/SceneGlobalData.h>
#include <DeepSea/Scene/SceneLoadContext.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>
#include <string.h>

typedef struct dsLightData
{
	dsSceneGlobalData globalData;
	dsShaderVariableGroup* variableGroup;
	dsVector3f direction;
	uint32_t nameID;
} dsLightData;

bool dsLightData_populateData(dsSceneGlobalData* globalData, const dsView* view,
	dsCommandBuffer* commandBuffer)
{
	dsLightData* lightData = (dsLightData*)globalData;
	dsVector4f direction =
		{{lightData->direction.x, lightData->direction.y, lightData->direction.z, 0.0f}};
	dsVector4f viewDirection;
	dsMatrix44_transform(viewDirection, view->viewMatrix, direction);
	DS_VERIFY(dsShaderVariableGroup_setElementData(lightData->variableGroup, 0, &viewDirection,
		dsMaterialType_Vec3, 0, 1));
	if (!dsShaderVariableGroup_commit(lightData->variableGroup, commandBuffer))
		return false;

	DS_VERIFY(dsSharedMaterialValues_setVariableGroupID(view->globalValues, lightData->nameID,
		lightData->variableGroup));
	return true;
}

bool dsLightData_destroy(dsSceneGlobalData* globalData)
{
	dsLightData* lightData = (dsLightData*)globalData;
	if (!dsShaderVariableGroup_destroy(lightData->variableGroup))
		return false;

	if (globalData->allocator)
		DS_VERIFY(dsAllocator_free(globalData->allocator, globalData));
	return true;
}

dsSceneGlobalData* dsLightData_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator*, void*,
	const uint8_t* data, size_t dataSize)
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
	dsSceneGlobalData* lightData =
		dsLightData_create(allocator, renderer->resourceManager, groupDesc);
	if (!lightData)
		return nullptr;

	dsLightData_setDirection(lightData, &direction);
	dsLightData_setColor(lightData, &color);
	dsLightData_setAmbientColor(lightData, &ambient);
	return lightData;
}

dsSceneGlobalData* dsLightData_create(dsAllocator* allocator,
	dsResourceManager* resourceManager, const dsShaderVariableGroupDesc* lightDesc)
{
	DS_ASSERT(allocator);
	DS_ASSERT(resourceManager);
	DS_ASSERT(lightDesc);

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsLightData)) +
		dsShaderVariableGroup_fullAllocSize(resourceManager, lightDesc);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return nullptr;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsLightData* lightData = DS_ALLOCATE_OBJECT(&bufferAlloc, dsLightData);
	DS_VERIFY(lightData);
	dsSceneGlobalData* globalData = (dsSceneGlobalData*)lightData;

	globalData->allocator = dsAllocator_keepPointer(allocator);
	globalData->valueCount = 1;
	globalData->populateDataFunc = &dsLightData_populateData;
	globalData->finishFunc = nullptr;
	globalData->destroyFunc = &dsLightData_destroy;

	lightData->variableGroup = dsShaderVariableGroup_create(resourceManager,
		(dsAllocator*)&bufferAlloc, allocator, lightDesc);
	if (!lightData->variableGroup)
	{
		if (allocator->freeFunc)
			DS_VERIFY(dsAllocator_free(allocator, buffer));
		return nullptr;
	}

	const char* name = "LightData";
	lightData->nameID = dsHashString(name);

	return globalData;
}

void dsLightData_setDirection(dsSceneGlobalData* globalData, const dsVector3f* direction)
{
	DS_VERIFY(globalData);
	DS_VERIFY(direction);
	dsLightData* lightData = (dsLightData*)globalData;
	dsVector3f_normalize(&lightData->direction, direction);
}

void dsLightData_setColor(dsSceneGlobalData* globalData, const dsVector3f* color)
{
	DS_VERIFY(globalData);
	DS_VERIFY(color);
	dsLightData* lightData = (dsLightData*)globalData;
	DS_VERIFY(dsShaderVariableGroup_setElementData(lightData->variableGroup, 1, color,
		dsMaterialType_Vec3, 0, 1));
}

void dsLightData_setAmbientColor(dsSceneGlobalData* globalData, const dsVector3f* color)
{
	DS_VERIFY(globalData);
	DS_VERIFY(color);
	dsLightData* lightData = (dsLightData*)globalData;
	DS_VERIFY(dsShaderVariableGroup_setElementData(lightData->variableGroup, 2, color,
		dsMaterialType_Vec3, 0, 1));
}
