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
#include <DeepSea/Core/Types.h>
#include <DeepSea/Scene/Types.h>
#include <DeepSea/SceneLighting/Export.h>
#include <DeepSea/SceneLighting/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions to create and manipulate scene shadows managers.
 * @see dsSceneShadowManager
 */

/**
 * @brief The type name for scene shadow manager.
 */
DS_SCENELIGHTING_EXPORT extern const char* const dsSceneShadowManager_typeName;

/**
 * @brief Gets the type for the dsShadowManager custom type for storage in dsSceneResources.
 * @return The custom type.
 */
DS_SCENELIGHTING_EXPORT const dsCustomSceneResourceType* dsSceneShadowManager_type(void);

/**
 * @brief Creates a shadow manager with the light shadows it manages.
 * @remark errno will be set on failure.
 * @param allocator The allocator for the shadow manager.
 * @param lightShadows The light shadows to manage. This takes ownership of the dsSceneLightShadows
 *     instances, while the array itself will not be kept. The instances will be destroyed even if
 *     creation of the shadow manager fails. Each light shadows must have a unique name, and the
 *     light names must either be unset or unique.
 * @param lightShadowsCount The number of light shadows.
 * @return The shadow manager or NULL if creation failed.
 */
DS_SCENELIGHTING_EXPORT dsSceneShadowManager* dsSceneShadowManager_create(dsAllocator* allocator,
	dsSceneLightShadows* const* lightShadows, uint32_t lightShadowsCount);

/**
 * @brief Finds light shadows by its name.
 * @param shadowManager The shadow manager.
 * @param name The name of the light shadows. This may not be the name of the light the shadows are
 *     associated with.
 * @return The light shadows or NULL if not found.
 */
DS_SCENELIGHTING_EXPORT dsSceneLightShadows* dsSceneShadowManager_findLightShadows(
	const dsSceneShadowManager* shadowManager, const char* name);

/**
 * @brief Finds the shadows associated with a light.
 * @param shadowManager The shadow manager.
 * @param lightName The name of the light.
 * @return The light shadows or NULL if not found.
 */
DS_SCENELIGHTING_EXPORT dsSceneLightShadows* dsSceneShadowManager_findShadowsForLightName(
	const dsSceneShadowManager* shadowManager, const char* lightName);

/**
 * @brief Finds the shadows associated with a light.
 * @param shadowManager The shadow manager.
 * @param lightID The ID of the light.
 * @return The light shadows or NULL if not found.
 */
DS_SCENELIGHTING_EXPORT dsSceneLightShadows* dsSceneShadowManager_findShadowsForLightID(
	const dsSceneShadowManager* shadowManager, uint32_t lightID);

/**
 * @brief Sets the light associated with scene light shadows.
 * @remark errno will be set on failure.
 * @param shadowManager The shadow manager.
 * @param lightShadows The light shadows to set the light name on.
 * @param lightName The name of the light, or NULL to disassociate with a light.
 * @return False if the light couldn't be set on the shadows.
 */
DS_SCENELIGHTING_EXPORT bool dsSceneShadowManager_setShadowsLightName(
	dsSceneShadowManager* shadowManager, dsSceneLightShadows* lightShadows,
	const char* lightName);

/**
 * @brief Sets the light associated with scene light shadows.
 * @remark errno will be set on failure.
 * @param shadowManager The shadow manager.
 * @param lightShadows The light shadows to set the light name on.
 * @param lightID The ID of the light, or 0 to disassocite with a light.
 * @return False if the light couldn't be set on the shadows.
 */
DS_SCENELIGHTING_EXPORT bool dsSceneShadowManager_setShadowsLightID(
	dsSceneShadowManager* shadowManager, dsSceneLightShadows* lightShadows, uint32_t lightID);

/**
 * @brief Prepares all the light shadows in the shadow manager that are associated with a light for
 *     the next frame.
 * @remark errno will be set on failure.
 * @param shadowManager The shadow manager.
 * @param view The view to prepare for.
 * @return False if an error occurred.
 */
DS_SCENELIGHTING_EXPORT bool dsSceneShadowManager_prepare(dsSceneShadowManager* shadowManager,
	const dsView* view);

/**
 * @brief Gets the number of global transform groups in the light shadows within the shadow manager.
 * @param shadowManager The shadow manager.
 * @return The number of global transform groups.
 */
DS_SCENELIGHTING_EXPORT uint32_t dsSceneShadowManager_globalTransformGroupCount(
	const dsSceneShadowManager* shadowManager);

/**
 * @brief Destroys a shadow manager.
 * @remark errno will be set on failure.
 * @param shadowManager The shadow manager to destroy.
 * @return False if the shadow manager couldn't be destroyed.
 */
DS_SCENELIGHTING_EXPORT bool dsSceneShadowManager_destroy(dsSceneShadowManager* shadowManager);

#ifdef __cplusplus
}
#endif
