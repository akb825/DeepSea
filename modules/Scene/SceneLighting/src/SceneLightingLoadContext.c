/*
 * Copyright 2020-2021 Aaron Barany
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

#include <DeepSea/SceneLighting/SceneLightingLoadContext.h>

#include "DeferredLightResolveLoad.h"
#include "InstanceForwardLightDataLoad.h"
#include "SceneLightSetLoad.h"
#include "SceneLightSetPrepareLoad.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Scene/SceneLoadContext.h>
#include <DeepSea/SceneLighting/DeferredLightResolve.h>
#include <DeepSea/SceneLighting/InstanceForwardLightData.h>
#include <DeepSea/SceneLighting/SceneLightSet.h>
#include <DeepSea/SceneLighting/SceneLightSetPrepare.h>

static bool destroySceneLightSet(void* lightSet)
{
	dsSceneLightSet_destroy((dsSceneLightSet*)lightSet);
	return true;
}

bool dsSceneLightingLoadConext_registerTypes(dsSceneLoadContext* loadContext)
{
	if (!loadContext)
	{
		errno = EINVAL;
		return false;
	}

	if (!dsSceneLoadContext_registerCustomSceneResourceType(loadContext, dsSceneLightSet_typeName,
			dsSceneLightSet_type(), &dsSceneLightSet_load, &destroySceneLightSet, NULL, NULL, 0))
	{
		return false;
	}

	if (!dsSceneLoadContext_registerGlobalDataType(loadContext,
			dsSceneLightSetPrepare_typeName, &dsSceneLightSetPrepare_load, NULL, NULL))
	{
		return false;
	}

	if (!dsSceneLoadContext_registerInstanceDataType(loadContext,
			dsInstanceForwardLightData_typeName, &dsInstanceForwardLightData_load, NULL, NULL))
	{
		return false;
	}

	if (!dsSceneLoadContext_registerItemListType(loadContext,
			dsDeferredLightResolve_typeName, &dsDeferredLightResolve_load, NULL, NULL))
	{
		return false;
	}

	return true;
}
