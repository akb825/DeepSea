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

#include <DeepSea/Scene/SceneCullManager.h>

#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Atomic.h>
#include <string.h>

bool dsSceneCullManager_reset(dsSceneCullManager* cullManager)
{
	if (!cullManager)
	{
		errno = EINVAL;
		return false;
	}

	memset(cullManager, 0, sizeof(dsSceneCullManager));
	return true;
}

uint32_t dsSceneCullManager_registerCullID(dsSceneCullManager* cullManager, dsSceneCullID cullID)
{
	if (!cullManager || !cullID)
	{
		errno = EINVAL;
		return DS_NO_SCENE_CULL;
	}

	DS_ASSERT(cullManager->registeredIDCount <= DS_ARRAY_SIZE(cullManager->cullIDs));
	for (uint32_t i = 0; i < cullManager->registeredIDCount; ++i)
	{
		if (cullManager->cullIDs[i] == cullID)
			return 1 << i;
	}

	if (cullManager->registeredIDCount >= DS_ARRAY_SIZE(cullManager->cullIDs))
	{
		errno = ENOMEM;
		return DS_NO_SCENE_CULL;
	}

	uint32_t index = cullManager->registeredIDCount++;
	cullManager->cullIDs[index] = cullID;
	return 1 << index;
}

uint32_t dsSceneCullManager_findCullID(const dsSceneCullManager* cullManager, dsSceneCullID cullID)
{
	if (!cullManager)
		return DS_NO_SCENE_CULL;

	for (uint32_t i = 0; i < cullManager->registeredIDCount; ++i)
	{
		if (cullManager->cullIDs[i] == cullID)
			return 1 << i;
	}

	return DS_NO_SCENE_CULL;
}

void dsSceneCullManager_setCullResult(uint32_t* mask, uint32_t instance, bool result);
