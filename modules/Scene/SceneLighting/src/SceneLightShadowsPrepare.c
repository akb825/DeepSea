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

#include <DeepSea/SceneLighting/SceneLightShadowsPrepare.h>

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/SceneLighting/SceneLightShadows.h>

#include <string.h>

typedef struct dsSceneLightShadowsPrepare
{
	dsSceneGlobalData globalData;
	dsSceneLightShadows* shadows;
	uint32_t transformGroupID;
} dsSceneLightShadowsPrepare;

const char* const dsSceneLightShadowsPrepare_typeName = "LightShadowsPrepare";

bool dsSceneLightShadowsPrepare_populateData(dsSceneGlobalData* globalData,
	const dsView* view, dsCommandBuffer* commandBuffer)
{
	DS_UNUSED(commandBuffer);
	dsSceneLightShadowsPrepare* prepare = (dsSceneLightShadowsPrepare*)globalData;
	return dsSceneLightShadows_prepare(prepare->shadows, view, prepare->transformGroupID);
}

bool dsSceneLightShadowsPrepare_destroyGlobalData(dsSceneGlobalData* globalData)
{
	if (!globalData->allocator)
		return true;

	DS_VERIFY(dsAllocator_free(globalData->allocator, globalData));
	return true;
}

dsSceneGlobalData* dsSceneLightShadowsPrepare_create(dsAllocator* allocator,
	dsSceneLightShadows* shadows, const char* transformGroupName)
{
	if (!allocator || !shadows || !transformGroupName)
	{
		errno = EINVAL;
		return NULL;
	}

	dsSceneLightShadowsPrepare* prepare = DS_ALLOCATE_OBJECT(allocator, dsSceneLightShadowsPrepare);
	if (!prepare)
		return NULL;

	dsSceneGlobalData* globalData = (dsSceneGlobalData*)prepare;
	globalData->allocator = dsAllocator_keepPointer(allocator);
	globalData->valueCount = 1;
	globalData->populateDataFunc = &dsSceneLightShadowsPrepare_populateData;
	globalData->finishFunc = NULL;
	globalData->destroyFunc = dsSceneLightShadowsPrepare_destroyGlobalData;

	prepare->shadows = shadows;
	prepare->transformGroupID = dsHashString(transformGroupName);

	return globalData;
}
