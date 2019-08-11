/*
 * Copyright 2019 Aaron Barany
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

#include <DeepSea/Scene/SceneGlobalData.h>
#include <DeepSea/Core/Error.h>

bool dsSceneGlobalData_populateData(dsSceneGlobalData* globalData, const dsView* view)
{
	if (!globalData || !globalData->populateDataFunc || !view)
	{
		errno = EINVAL;
		return false;
	}

	return globalData->populateDataFunc(globalData, view);
}

bool dsSceneGlobalData_finish(dsSceneGlobalData* globalData)
{
	if (!globalData || !globalData->finishFunc)
	{
		errno = EINVAL;
		return false;
	}

	return globalData->finishFunc(globalData);
}

bool dsSceneGlobalData_destroy(dsSceneGlobalData* globalData)
{
	if (!globalData || !globalData->destroyFunc)
		return true;

	return globalData->destroyFunc(globalData);
}
