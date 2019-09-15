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

#pragma once

#include <DeepSea/Core/Config.h>

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Scene/Export.h>
#include <DeepSea/Scene/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating cull managers.
 * @see dsSceneCullManager
 */

/**
 * @brief Constant for no cull instance.
 */
#define DS_NO_SCENE_CULL (uint32_t)-1

/**
 * @brief Resets the cull manager to be empty.
 * @remark errno will be set on failure.
 * @param cullManager The cull manager.
 * @return False if cullManager is NULL.
 */
DS_SCENE_EXPORT bool dsSceneCullManager_reset(dsSceneCullManager* cullManager);

/**
 * @brief Registers a cull ID with the manager.
 * @remark errno will be set on failure.
 * @param cullManager The cull manager.
 * @param cullID The cull ID to register.
 * @return The cull mask or DS_NO_SCENE_CULL if it couldn't be registered.
 */
DS_SCENE_EXPORT uint32_t dsSceneCullManager_registerCullID(dsSceneCullManager* cullManager,
	dsSceneCullID cullID);

/**
 * @brief Finds a previously registered cull ID.
 * @param cullManager The cull manager.
 * @param cullID The cull ID to find.
 * @return The cull mask or DS_NO_SCENE_CULL if not present.
 */
DS_SCENE_EXPORT uint32_t dsSceneCullManager_findCullID(const dsSceneCullManager* cullManager,
	dsSceneCullID cullID);

/**
 * @brief Sets the cull result.
 * @remark This function is thread-safe, allowing for cull checks to be done asynchronously.
 * @param[inout] mask The mask to set the result on.
 * @param instance The instance of the cull as found by dsSceneCullManager_registerCullID() or
 *     dsSceneCullManager_findCullID(). It is assumed that it is not DS_NO_SCENE_CULL.
 * @param result True if the item culled is visible, false if not visible.
 */
DS_SCENE_EXPORT inline void dsSceneCullManager_setCullResult(uint32_t* mask, uint32_t instance,
	bool result);

/**
 * @brief Gets the cull result.
 * @param mask The mask to get the result from.
 * @param instance The index of the cull as found by dsSceneCullManager_registerCullID() or
 *     dsSceneCullManager_findCullID(). It is assumed that it is not DS_NO_SCENE_CULL.
 * @return True if the item culled is visible, false if not visible.
 */
DS_SCENE_EXPORT inline bool dsSceneCullManager_getCullResult(uint32_t mask, uint32_t instance);

inline void dsSceneCullManager_setCullResult(uint32_t* mask, uint32_t instance, bool result)
{
	DS_ASSERT(mask);
	DS_ASSERT(instance != DS_NO_SCENE_CULL);
	DS_ASSERT(result == 0 || result == 1);

	uint32_t andVal = ~instance;
	uint32_t orVal = result ? instance : 0;

	uint32_t expectedMask, newMask;
	DS_ATOMIC_LOAD32(mask, &expectedMask);
	do
	{
		newMask = (expectedMask & andVal) | orVal;
	} while (!DS_ATOMIC_COMPARE_EXCHANGE32(mask, &expectedMask, &newMask, true));
}

inline bool dsSceneCullManager_getCullResult(uint32_t mask, uint32_t instance)
{
	DS_ASSERT(instance != DS_NO_SCENE_CULL);
	return (mask & instance) != 0;
}

#ifdef __cplusplus
}
#endif
