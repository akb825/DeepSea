/*
 * Copyright 2021-2022 Aaron Barany
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
#include <DeepSea/SceneLighting/Export.h>
#include <DeepSea/SceneLighting/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating scene shadow manager prepares.
 *
 * This will prepare a shadow manager for use in the scene. This a scene item list object fit into
 * the scene layout.
 *
 * @remark This item list type contains global values and must be a alone as an entry in the
 *     sharedItemlist of a dsScene.
 */

/**
 * @brief The scene shadow manager prepare type name.
 */
DS_SCENELIGHTING_EXPORT extern const char* const dsSceneShadowManagerPrepare_typeName;

/**
 * @brief Gets the type of a shadow manager prepare.
 * @return The type of a shadow manager prepare.
 */
DS_SCENELIGHTING_EXPORT dsSceneItemListType dsSceneShadowManagerPrepare_type(void);

/**
 * @brief Creates a scene shadow manager prepare.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the light shadows prepare with.
 * @param name The name of the shadow manager prepare. This will be copied.
 * @param shadowManager The scene shadow manager to prepare.
 * @return The scene shadow manager prepare or NULL if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT dsSceneItemList* dsSceneShadowManagerPrepare_create(
	dsAllocator* allocator, const char* name, dsSceneShadowManager* shadowManager);

#ifdef __cplusplus
}
#endif

