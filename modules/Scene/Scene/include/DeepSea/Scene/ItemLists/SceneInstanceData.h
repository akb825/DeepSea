/*
 * Copyright 2019-2023 Aaron Barany
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
#include <DeepSea/Scene/Types.h>
#include <DeepSea/Scene/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating scene instance data.
 *
 * Creation and destruction of scene instance data, as well as populating of data, must be performed
 * on the main thread or on a thread with an active resource context. Usage should not be done
 * simultaneously across multiple threads, which in practice usually means to use separate instances
 * across multiple dsSceneItemList objects.
 *
 * @see dsSceneInstanceData
 */

/**
 * @brief Loads a scene instance data from a flatbuffer data buffer.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the scene nodes.
 * @param resourceAllocator The allocator to create graphics resources with. If NULL, it will use
 *     the instance data allocator.
 * @param loadContext The scene load context.
 * @param scratchData The scene scratch data.
 * @param type The type name of the node to load.
 * @param data The data for the instance data. The data isn't used after this call.
 * @param size The size of the data buffer.
 * @return The instance data, or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsSceneInstanceData* dsSceneInstanceData_load(dsAllocator* allocator,
	dsAllocator* resourceAllocator, const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, const char* type, const void* data, size_t size);

/**
 * @brief Populates the instance data.
 * @remark errno will be set on failure.
 * @param instanceData The instance data.
 * @param view The view being drawn.
 * @param commandBuffer The command buffer to process GPU operations. This may be NULL if
 *     needsCommandBuffer is false.
 * @param instances The instances that will be drawn.
 * @param instanceCount The number of instances.
 * @return False if the data couldn't be set.
 */
DS_SCENE_EXPORT bool dsSceneInstanceData_populateData(dsSceneInstanceData* instanceData,
	const dsView* view, dsCommandBuffer* commandBuffer, const dsSceneTreeNode* const* instances,
	uint32_t instanceCount);

/**
 * @brief Binds the data for an instance.
 * @remark errno will be set on failure.
 * @param instanceData The instance data.
 * @param index The index for the instance to bind.
 * @param values The values to bind to.
 * @return False if an error occurred.
 */
DS_SCENE_EXPORT bool dsSceneInstanceData_bindInstance(dsSceneInstanceData* instanceData,
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
 * @brief Gets the hash for a scene instance data.
 * @param instanceData The instance data to get the hash for.
 * @param seed The seed for the hash, useful when this is used as part of a parent object such as
 *     dsSceneItemList. Set to DS_DEFAULT_HASH_SEED if computing the has in isolation.
 * @return The hash for the instance data.
 */
DS_SCENE_EXPORT uint32_t dsSceneInstanceData_hash(
	const dsSceneInstanceData* instanceData, uint32_t seed);

/**
 * @brief Checks whether two item scene instance datas are equal.
 * @param left The left hand side.
 * @param right The right hand side.
 * @return Whether left and right are equal.
 */
DS_SCENE_EXPORT bool dsSceneInstanceData_equal(
	const dsSceneInstanceData* left, const dsSceneInstanceData* right);

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
