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

#include <DeepSea/SceneLighting/SceneShadowManagerPrepare.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/SceneLighting/SceneLightShadows.h>
#include <DeepSea/SceneLighting/SceneShadowManager.h>

#include <string.h>

typedef struct dsSceneShadowManagerPrepare
{
	dsSceneGlobalData globalData;
	dsSceneShadowManager* shadowManager;
} dsSceneLightShadowsPrepare;

const char* const dsSceneShadowManagerPrepare_typeName = "ShadowManagerPrepare";

bool dsSceneShadowManagerPrepare_populateData(dsSceneGlobalData* globalData,
	const dsView* view, dsCommandBuffer* commandBuffer)
{
	DS_UNUSED(commandBuffer);
	dsSceneLightShadowsPrepare* prepare = (dsSceneLightShadowsPrepare*)globalData;
	return dsSceneShadowManager_prepare(prepare->shadowManager, view);
}

bool dsSceneShadowManagerPrepare_destroyGlobalData(dsSceneGlobalData* globalData)
{
	if (!globalData->allocator)
		return true;

	DS_VERIFY(dsAllocator_free(globalData->allocator, globalData));
	return true;
}

dsSceneGlobalData* dsSceneShadowManagerPrepare_create(dsAllocator* allocator,
	dsSceneShadowManager* shadowManager)
{
	if (!allocator || !shadowManager)
	{
		errno = EINVAL;
		return NULL;
	}

	dsSceneLightShadowsPrepare* prepare = DS_ALLOCATE_OBJECT(allocator, dsSceneLightShadowsPrepare);
	if (!prepare)
		return NULL;

	dsSceneGlobalData* globalData = (dsSceneGlobalData*)prepare;
	globalData->allocator = dsAllocator_keepPointer(allocator);
	globalData->valueCount = dsSceneShadowManager_globalTransformGroupCount(shadowManager);
	globalData->populateDataFunc = &dsSceneShadowManagerPrepare_populateData;
	globalData->finishFunc = NULL;
	globalData->destroyFunc = dsSceneShadowManagerPrepare_destroyGlobalData;

	prepare->shadowManager = shadowManager;

	return globalData;
}
