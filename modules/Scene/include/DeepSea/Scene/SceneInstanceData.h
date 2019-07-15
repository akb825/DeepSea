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
#include <DeepSea/Scene/Export.h>
#include <DeepSea/Scene/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating scene instance data.
 *
 * Creation and destruction of scene instance data, as well as reserving space, must be performed on
 * the main thread or on a thread with an active resource context. Usage should not be done
 * simultaneously across multiple threads, which in practice usually means to use separate instances
 * across multiple dsSceneModelList objects.
 *
 * @see dsSceneInstanceData
 */

/**
 * @brief Creates a scene instance data object.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the data with. This must support freeing memory.
 * @param resourceManager The resource manager to create any resources with.
 * @param dataDesc The description for the data held for each instance. This must remain alive at
 *     least as long as the instance data object.
 * @param nameID The name ID to use when setting the buffer data on the dsSharedMaterialValues
 *     instance.
 * @param populateDataFunc Function to populate the instance data.
 * @param userData The user data that will be provided to populateDataFunc. This may be NULL.
 * @param destroyUserDataFunc Function to destroy the user data. This may be NULL. This will be
 *     called if creation fails.
 * @return The instance data or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsSceneInstanceData* dsSceneInstanceData_create(dsAllocator* allocator,
	dsResourceManager* resourceManager, const dsShaderVariableGroupDesc* dataDesc, uint32_t nameID,
	dsPopulateSceneInstanceDataFunction populateDataFunc, void* userData,
	dsDestroySceneInstanceUserDataFunction destroyUserDataFunc);

/**
 * @brief Populates the instance data.
 * @remark errno will be set on failure.
 * @param instanceData The instance data.
 * @param view The view being drawn.
 * @param instances The instances that will be drawn.
 * @param instanceCount The number of instances.
 * @return False if the data couldn't be set.
 */
DS_SCENE_EXPORT bool dsSceneInstanceData_populateData(dsSceneInstanceData* instanceData,
	const dsView* view, const dsSceneInstanceInfo* instances, uint32_t instanceCount);

/**
 * @brief Binds the data for an instance.
 * @remark errno will be set on failure.
 * @param instanceData The instance data.
 * @param index The index for the instance to bind.
 * @param values The values to bind to.
 * @return False if an error occurred.
 */
DS_SCENE_EXPORT bool dsSceneInstanceData_bindInstance(const dsSceneInstanceData* instanceData,
	uint32_t index, dsSharedMaterialValues* values);

/**
 * @brief Finishes the current set of instance data.
 *
 * This should be called after drawing with the instance data has been queued.
 *
 * @remark errno will be set on failure.
 * @param instanceData The instance data.
 * @return False if an error occurred.
 */
DS_SCENE_EXPORT bool dsSceneInstanceData_finish(dsSceneInstanceData* instanceData);

/**
 * @brief Destroys a scene instance data object.
 * @remark errno will be set on failure.
 * @param instanceData The instance data to destroy.
 * @return False if the instance data couldn't be destroyed.
 */
DS_SCENE_EXPORT bool dsSceneInstanceData_destroy(dsSceneInstanceData* instanceData);

#ifdef __cplusplus
}
#endif

