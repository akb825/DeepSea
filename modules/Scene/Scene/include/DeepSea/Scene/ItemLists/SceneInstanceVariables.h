/*
 * Copyright 2019-2025 Aaron Barany
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
#include <DeepSea/Scene/ItemLists/Types.h>
#include <DeepSea/Scene/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating scene instance variables.
 * @see dsSceneInstanceData
 */

/**
 * @brief Gets the type of scene instance variables.
 * @return The type of scene instance variables.
 */
DS_SCENE_EXPORT const dsSceneInstanceDataType* dsSceneInstanceVariables_type(void);

/**
 * @brief Creates a scene instance variables object.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the data with. This must support freeing memory.
 * @param resourceAllocator The allocator to create graphics resources with. If NULL this will
 *     default to allocator.
 * @param resourceManager The resource manager to create graphics resources with.
 * @param dataDesc The description for the data held for each instance. This must remain alive at
 *     least as long as the instance data object.
 * @param nameID The name ID to use when setting the buffer data on the dsSharedMaterialValues
 *     instance.
 * @param instanceVariablesType The type for the instance variables.
 * @param userData The user data that will be provided to populateDataFunc. This may be NULL.
 */
DS_SCENE_EXPORT dsSceneInstanceData* dsSceneInstanceVariables_create(dsAllocator* allocator,
	dsAllocator* resourceAllocator, dsResourceManager* resourceManager,
	const dsShaderVariableGroupDesc* dataDesc, uint32_t nameID,
	const dsSceneInstanceVariablesType* instanceVariablesType, void* userData);

#ifdef __cplusplus
}
#endif

