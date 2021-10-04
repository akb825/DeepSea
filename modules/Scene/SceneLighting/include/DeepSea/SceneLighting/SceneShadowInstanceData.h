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
 * @brief Functions for creating dsSceneInstanceData instances that manage binding shadow transform
 * groups.
 *
 * This is typically used for binding shadow transforms for shadow casters of deferred lights, where
 * instance bindings are used in place of global bindings.
 *
 * @see dsSceneInstanceData
 * @see dsShadowManager
 */

/**
 * @brief The scene shadow instance data type name.
 */
DS_SCENELIGHTING_EXPORT extern const char* const dsSceneShadowInstanceData_typeName;

/**
 * @brief Creates scene shadow instance data to use with a dsSceneItemList.
 * @param allocator The allocator to create the forward light data with.
 * @param shadows The scene shadows to bind the instance for.
 * @param transformGroupName The name of the transform group to bind.
 * @return The instance data or NULL if an error occurred.
 */
DS_SCENELIGHTING_EXPORT dsSceneInstanceData* dsSceneShadowInstanceData_create(
	dsAllocator* allocator, const dsSceneLightShadows* shadows, const char* transformGroupName);

#ifdef __cplusplus
}
#endif
