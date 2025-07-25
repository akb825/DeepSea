/*
 * Copyright 2020-2025 Aaron Barany
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
 * @brief Functions for creating and manipulating scene light set prepares.
 * @see dsSceneLightSetPrepare
 */

/**
 * @brief The scene light set prepare type name.
 */
DS_SCENELIGHTING_EXPORT extern const char* const dsSceneLightSetPrepare_typeName;

/**
 * @brief Gets the type of a light set prepare.
 * @return The type of a light set prepare.
 */
DS_SCENELIGHTING_EXPORT const dsSceneItemListType* dsSceneLightSetPrepare_type(void);

/**
 * @brief Creates a scene light set prepare.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the light set prepare with. This must support freeing
 *     memory.
 * @param name The name of the light set prepare. This will be copied.
 * @param lightSet The light set to prepare.
 * @param intensityThreshold The threshold below which the light is considered out of view. This
 *     must be > 0. Use DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD for the default value.
 * @return The scene light set prepare or NULL if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT dsSceneLightSetPrepare* dsSceneLightSetPrepare_create(
	dsAllocator* allocator, const char* name, dsSceneLightSet* lightSet,
	float intensityThreshold);

/**
 * @brief Gets the light set the scene light set prepare manages.
 * @remark errno will be set on failure.
 * @param prepare The scene light set prepare.
 * @return The scene light set or NULL if prepare is NULL.
 */
DS_SCENELIGHTING_EXPORT const dsSceneLightSet* dsSceneLightSetPrepare_getLightSet(
	const dsSceneLightSetPrepare* prepare);

/**
 * @brief Gets the intensity threshold.
 * @remark errno will be set on failure.
 * @param prepare The scene light prepare.
 * @return The intensity threshold or 0 if prepare is NULL.
 */
DS_SCENELIGHTING_EXPORT float dsSceneLightSetPrepare_getIntensityThreshold(
	const dsSceneLightSetPrepare* prepare);

/**
 * @brief Sets the intensity threshold.
 * @remark errno will be set on failure.
 * @param prepare The scene light prepare.
 * @param intensityThreshold The threshold below which the light is considered out of view. This
 *     must be > 0. Use DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD for the default value.
 * @return False if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT bool dsSceneLightSetPrepare_setIntensityThreshold(
	dsSceneLightSetPrepare* prepare, float intensityThreshold);

/**
 * @brief Destroys a scene light prepare.
 * @param prepare The scene light prepare to destroy.
 */
DS_SCENELIGHTING_EXPORT void dsSceneLightSetPrepare_destroy(dsSceneLightSetPrepare* prepare);

#ifdef __cplusplus
}
#endif

