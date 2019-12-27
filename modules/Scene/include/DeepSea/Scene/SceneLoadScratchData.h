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
#include <DeepSea/Core/Streams/Types.h>
#include <DeepSea/Scene/Export.h>
#include <DeepSea/Scene/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating scene load scratch data.
 * @see dsSceneLoadScratchData
 */

/**
 * @brief Creates load scratch data.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the scratch data with. This must support freeing memory.
 * @param commandBuffer The command buffer to use for commands related to loading. This may be NULL,
 *     but must be set before loading any shader variable group resources with embedded data.
 * @return The load scratch data.
 */
DS_SCENE_EXPORT dsSceneLoadScratchData* dsSceneLoadScratchData_create(dsAllocator* allocator,
	dsCommandBuffer* commandBuffer);

/**
 * @brief Allocates scratch data.
 * @remark errno will be set on failure.
 * @param scratchData The scratch data to allocate from.
 * @param size The size to allocator.
 * @return The allocated data, or NULL if allocation failed.
 */
DS_SCENE_EXPORT void* dsSceneLoadScratchData_allocate(dsSceneLoadScratchData* scratchData,
	uint32_t size);

/**
 * @brief Reads data from the stream until its end.
 * @param[out] outSize The size read from the stream. This should be popped after usage.
 * @param scratchData The scratch data.
 * @param stream The stream to read from.
 * @return The data, or NULL if it couldn't be allocated or read.
 */
DS_SCENE_EXPORT void* dsSceneLoadScratchData_readUntilEnd(uint32_t* outSize,
	dsSceneLoadScratchData* scratchData, dsStream* stream);

/**
 * @brief Pops allocated data from the scratch data.
 *
 * This deallocates the last allocated memory.
 *
 * @remark errno will be set on failure.
 * @param scratchData The scratch data to pop from.
 * @param size The number of bytes from the previous allocation to remove.
 * @return False if the parameters are invalid.
 */
DS_SCENE_EXPORT bool dsSceneLoadScratchData_popData(dsSceneLoadScratchData* scratchData,
	uint32_t size);

/**
 * @brief Pushes scene resources to be used during load.
 * @remark errno will be set on failure.
 * @param scratchData The scratch data to push the resources on.
 * @param resources The scene resources to push.
 * @param resourceCount The number of scene resources.
 * @return False if the parameters are invalid.
 */
DS_SCENE_EXPORT bool dsSceneLoadScratchData_pushSceneResources(dsSceneLoadScratchData* scratchData,
	dsSceneResources** resources, uint32_t resourceCount);

/**
 * @brief Pops scene resources used during load.
 * @remark errno will be set on failure.
 * @param scratchData The scratch data to push the resources from.
 * @param resourceCount The number of resources to pop.
 */
DS_SCENE_EXPORT bool dsSceneLoadScratchData_popSceneResources(dsSceneLoadScratchData* scratchData,
	uint32_t resourceCount);

/**
 * @brief Gets the scene resources currently pushed on the scratch data.
 * @param[out] outResourceCount Output variable for the number of resources.
 * @param scratchData The scratch data to get the resources from.
 * return The scene resources.
 */
DS_SCENE_EXPORT dsSceneResources** dsSceneLoadScratchData_getSceneResources(
	uint32_t* outResourceCount, const dsSceneLoadScratchData* scratchData);

/**
 * @brief Finds a scene resource.
 *
 * This will search from the pushed resources in reverse order, such that the most recently pushed
 * scene resources will be checked first.
 *
 * @param[out] outType The type of the resource.
 * @param[out] outResource The pointer of the resource.
 * @param scratchData The scratch data to search in.
 * @param name The name of the resource.
 * @return True if the resource was found.
 */
DS_SCENE_EXPORT bool dsSceneLoadScratchData_findResource(dsSceneResourceType* outType,
	void** outResource, const dsSceneLoadScratchData* scratchData, const char* name);

/**
 * @brief Gets the command buffer to use for commands related to loading.
 * @param scratchData The scratch data to get the command buffer from.
 * @return The command buffer.
 */
DS_SCENE_EXPORT dsCommandBuffer* dsSceneLoadScratchData_getCommandBuffer(
	const dsSceneLoadScratchData* scratchData);

/**
 * @brief Sets the command buffer to use for commands related to loading.
 * @remark errno will be set on failure.
 * @param scratchData The scratch data to set the command buffer on.
 * @param commandBuffer The command buffer to set.
 * @return False if scratchData is NULL.
 */
DS_SCENE_EXPORT bool dsSceneLoadScratchData_setCommandBuffer( dsSceneLoadScratchData* scratchData,
	dsCommandBuffer* commandBuffer);

/**
 * @brief Destroys load scratch data.
 * @param scratchData The scratch data to destroy.
 * @return False if the parameters are invalid.
 */
DS_SCENE_EXPORT void dsSceneLoadScratchData_destroy(dsSceneLoadScratchData* scratchData);

#ifdef __cplusplus
}
#endif
