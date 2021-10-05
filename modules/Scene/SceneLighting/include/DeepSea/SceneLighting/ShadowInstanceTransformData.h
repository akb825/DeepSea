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
 * @file
 * @brief Functions for creating dsSceneInstanceData instances that manage transforms for shadow
 * casting.
 *
 * This populates the uniforms found in DeepSea/Scene/Shaders/InstanceTransform.mslh, typically
 * for rendering shadow casters.
 *
 * @see dsSceneInstanceData
 * @see dsInstanceTransformData
 * @see dsShadowManager
 */

/**
 * @brief The shadow instance transform data type name.
 */
DS_SCENELIGHTING_EXPORT extern const char* const dsShadowInstanceTransformData_typeName;

/**
 * @brief Creates scene shadow instance data to use with a dsSceneItemList.
 * @param allocator The allocator to create the forward light data with.
 * @param resourceManager The resource manager.
 * @param shadows The scene shadows to bind the transform for.
 * @param surface The index for the surface in the scene shadows to bind the transform for.
 * @param transformDesc The shader variable group description created from
 *     dsInstanceTransformData_createShaderVariableGroupDesc(). This must remain alive at least as
 *     long as the instance data object.
 * @return The instance data or NULL if an error occurred.
 */
DS_SCENELIGHTING_EXPORT dsSceneInstanceData* dsShadowInstanceTransformData_create(
	dsAllocator* allocator, dsResourceManager* resourceManager, const dsSceneLightShadows* shadows,
	uint32_t surface, const dsShaderVariableGroupDesc* transformDesc);

#ifdef __cplusplus
}
#endif
