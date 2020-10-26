/*
 * Copyright 2020 Aaron Barany
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
#include <DeepSea/SceneLighting/Export.h>
#include <DeepSea/SceneLighting/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief The default number of forward lights.
 */
#define DS_DEFAULT_FORWARD_LIGHT_COUNT 4

/**
 * @file
 * @brief Functions for creating dsSceneInstanceData instances that manage lights on instances for
 * forward lighting.
 *
 * This populates the uniforms found in DeepSea/SceneLighting/Shaders/ForwardLights.mslh. The
 * position of the instance's transform (i.e. the last column of the transform matrix) will be used
 * as the position for gathering the brightest lights.
 *
 * @see dsSceneInstanceData
 */

/**
 * @brief The instance forward light data type name.
 */
DS_SCENELIGHTING_EXPORT extern const char* const dsInstanceForwardLightData_typeName;

/**
 * @brief Creates the shader variable group description used to describe the variables for the
 *     instance forward lights.
 * @remark This should be shared among all dsInstanceForwardLightData instances that use the same
 *     number of lights.
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager.
 * @param allocator The allocator to create the shader variable group with. If NULL, the allocator
 *     from resourceManager.
 * @param lightCount The number of lights. Use DS_DEFAULT_FORWARD_LIGHT_COUNT for the default number
 *     of lights.
 * @return The shader variable group description or NULL if an error occurred.
 */
DS_SCENELIGHTING_EXPORT dsShaderVariableGroupDesc*
dsInstanceForwardLightData_createShaderVariableGroupDesc(dsResourceManager* resourceManager,
	dsAllocator* allocator, uint32_t lightCount);

/**
 * @brief Creates instance forward light data to use with a dsSceneItemList.
 * @param allocator The allocator to create the forward light data with.
 * @param resourceManager The resource manager.
 * @param lightDesc The shader variable group description created from
 *     dsInstanceForwardLightData_createShaderVariableGroupDesc(). This must remain alive at least
 *     as long as the instance data object.
 * @param lightSet The light set to gather lights from. This must remain alive at least as long as
 *     the instance data object.
 * @return The instance data or NULL if an error occurred.
 */
DS_SCENELIGHTING_EXPORT dsSceneInstanceData* dsInstanceForwardLightData_create(
	dsAllocator* allocator, dsResourceManager* resourceManager,
	const dsShaderVariableGroupDesc* lightDesc, const dsSceneLightSet* lightSet);

#ifdef __cplusplus
}
#endif

