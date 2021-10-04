/*
 * Copyright 2021 Aaron Barany
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

#include <DeepSea/SceneLighting/SceneShadowInstanceData.h>

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Scene/ItemLists/SceneInstanceData.h>
#include <DeepSea/SceneLighting/SceneLightShadows.h>

typedef struct dsSceneShadowInstanceData
{
	dsSceneInstanceData instanceData;
	const dsSceneLightShadows* shadows;
	uint32_t transformGroupID;
} dsSceneShadowInstanceData;

bool dsSceneShadowInstanceData_populateData(dsSceneInstanceData* instanceData,
	const dsView* view, const dsSceneInstanceInfo* instances, uint32_t instanceCount)
{
	DS_UNUSED(instanceData);
	DS_UNUSED(view);
	DS_UNUSED(instances);
	DS_UNUSED(instanceCount);
	return true;
}

bool dsSceneShadowInstanceData_bindInstance(dsSceneInstanceData* instanceData,
	uint32_t index, dsSharedMaterialValues* values)
{
	// Only need to bind once.
	if (index > 0)
		return true;

	dsSceneShadowInstanceData* shadowInstanceData = (dsSceneShadowInstanceData*)instanceData;
	return dsSceneLightShadows_bindTransformGroup(shadowInstanceData->shadows, values,
		shadowInstanceData->transformGroupID);
}

bool dsSceneShadowInstanceData_finish(dsSceneInstanceData* instanceData)
{
	DS_UNUSED(instanceData);
	return true;
}

bool dsSceneShadowInstanceData_destroy(dsSceneInstanceData* instanceData)
{
	if (instanceData->allocator)
		DS_VERIFY(dsAllocator_free(instanceData->allocator, instanceData));
	return true;
}

const char* const dsSceneShadowInstanceData_typeName = "ShadowInstanceData";

dsSceneInstanceData* dsSceneShadowInstanceData_create(
	dsAllocator* allocator, const dsSceneLightShadows* shadows, const char* transformGroupName)
{
	if (!allocator || !shadows || !transformGroupName)
	{
		errno = EINVAL;
		return NULL;
	}

	dsSceneShadowInstanceData* shadowInstanceData =
		DS_ALLOCATE_OBJECT(allocator, dsSceneShadowInstanceData);
	if (!shadowInstanceData)
		return NULL;

	dsSceneInstanceData* instanceData = (dsSceneInstanceData*)shadowInstanceData;
	instanceData->allocator = dsAllocator_keepPointer(allocator);
	instanceData->valueCount = 1;
	instanceData->populateDataFunc = &dsSceneShadowInstanceData_populateData;
	instanceData->bindInstanceFunc = &dsSceneShadowInstanceData_bindInstance;
	instanceData->finishFunc = &dsSceneShadowInstanceData_finish;
	instanceData->destroyFunc = &dsSceneShadowInstanceData_destroy;

	shadowInstanceData->shadows = shadows;
	shadowInstanceData->transformGroupID = dsHashString(transformGroupName);
	return instanceData;
}
