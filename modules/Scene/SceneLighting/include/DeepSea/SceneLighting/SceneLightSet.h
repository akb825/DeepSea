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
#include <DeepSea/Core/Types.h>
#include <DeepSea/Geometry/Types.h>
#include <DeepSea/Scene/Types.h>
#include <DeepSea/SceneLighting/Export.h>
#include <DeepSea/SceneLighting/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Functions for creating and manipulating scene light sets.
 *
 * Lookups into this will be frequent, so as a result the index is done by pre-hashing the name. You
 * may either access the lights by name or by the ID, which is the hash of the name. (by calling
 * dsHashString())
 *
 * @see dsSceneLightSet
 */

/**
 * @brief The type name for a scene light set.
 */
DS_SCENELIGHTING_EXPORT extern const char* const dsSceneLightSet_typeName;

/**
 * @brief Gets the type for the dsSceneLightSet custom type for storage in dsSceneResources.
 * @return The custom type.
 */
DS_SCENELIGHTING_EXPORT const dsCustomSceneResourceType* dsSceneLightSet_type(void);

/**
 * @brief Creates a light set.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the light set with. This must support freeing memory.
 * @param maxLights The maximum number of lights allowed in the set.
 * @param ambientColor The color of the ambient light.
 * @param ambientIntensity The intensity of the ambient light.
 * @return The created scene light set or NULL if it couldn't be created.
 */
DS_SCENELIGHTING_EXPORT dsSceneLightSet* dsSceneLightSet_create(dsAllocator* allocator,
	uint32_t maxLights, const dsColor3f* ambientColor, float ambientIntensity);

/**
 * @brief Creates a custom resource to wrap a dsSceneLightSet.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the custom resource.
 * @param lightSet The light set to wrap.
 * @return The custom resource or NULL if an error occurred.
 */
DS_SCENELIGHTING_EXPORT dsCustomSceneResource* dsSceneLightSet_createResource(
	dsAllocator* allocator, dsSceneLightSet* lightSet);

/**
 * @brief Gets the maximum number of lights that can be set.
 * @param lightSet The light set.
 * @return The maximum number of lights.
 */
DS_SCENELIGHTING_EXPORT uint32_t dsSceneLightSet_getMaxLights(const dsSceneLightSet* lightSet);

/**
 * @brief Gets the number of remaining lights that can be set.
 * @param lightSet The light set.
 * @return The number of remaining lights that can be set.
 */
DS_SCENELIGHTING_EXPORT uint32_t dsSceneLightSet_getRemainingLights(
	const dsSceneLightSet* lightSet);

/**
 * @brief Adds a light by name.
 * @remark errno will be set on failure.
 * @param lightSet The light set.
 * @param name The name for the light.
 * @return The added light or NULL if an error occurred. It is expected that the caller immediately
 *     populate the light values after this call.
 */
DS_SCENELIGHTING_EXPORT dsSceneLight* dsSceneLightSet_addLightName(dsSceneLightSet* lightSet,
	const char* name);

/**
 * @brief Adds a light by name ID.
 * @remark errno will be set on failure.
 * @param lightSet The light set.
 * @param nameID The hash of the name for the light.
 * @return The added light or NULL if an error occurred. It is expected that the caller immediately
 *     populate the light values after this call.
 */
DS_SCENELIGHTING_EXPORT dsSceneLight* dsSceneLightSet_addLightID(dsSceneLightSet* lightSet,
	uint32_t nameID);

/**
 * @brief Finds a previously added light by name.
 * @param lightSet The light set.
 * @param name The name of the light.
 * @return The found light or NULL if not found.
 */
DS_SCENELIGHTING_EXPORT dsSceneLight* dsSceneLightSet_findLightName(const dsSceneLightSet* lightSet,
	const char* name);

/**
 * @brief Finds a previously added light by name ID.
 * @param lightSet The light set.
 * @param nameID The hash of the name of the light.
 * @return The found light or NULL if not found.
 */
DS_SCENELIGHTING_EXPORT dsSceneLight* dsSceneLightSet_findLightID(const dsSceneLightSet* lightSet,
	uint32_t nameID);

/**
 * @brief Removes a previously added light by name.
 * @param lightSet The light set.
 * @param name The name of the light.
 * @return False if the light isn't present in the light set.
 */
DS_SCENELIGHTING_EXPORT bool dsSceneLightSet_removeLightName(dsSceneLightSet* lightSet,
	const char* name);

/**
 * @brief Removes a previously added light by name.
 * @param lightSet The light set.
 * @param nameID The hash of the name of the light.
 * @return False if the light isn't present in the light set.
 */
DS_SCENELIGHTING_EXPORT bool dsSceneLightSet_removeLightID(dsSceneLightSet* lightSet,
	uint32_t nameID);

/**
 * @brief Gets the ID for the main light.
 * @param lightSet The light set.
 * @return The ID of the main light or 0 if not set.
 */
DS_SCENELIGHTING_EXPORT uint32_t dsSceneLightSet_getMainLightID(const dsSceneLightSet* lightSet);

/**
 * @brief Sets the name for the main light.
 * @remark errno will be set on failure.
 * @param lightSet The light set.
 * @param name The name of the main light, or NULL to set no main light.
 * @return False if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT bool dsSceneLightSet_setMainLightName(dsSceneLightSet* lightSet,
	const char* name);

/**
 * @brief Sets the ID for the main light.
 * @remark errno will be set on failure.
 * @param lightSet The light set.
 * @param name*D The ID of the main light, or 0 to set no main light.
 * @return False if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT bool dsSceneLightSet_setMainLightID(dsSceneLightSet* lightSet,
	uint32_t nameID);

/**
 * @brief Clears all the lights a light set.
 * @remark errno will be set on failure.
 * @param lightSet The light set.
 * @return False if lightSet is NULL.
 */
DS_SCENELIGHTING_EXPORT bool dsSceneLightSet_clearLights(dsSceneLightSet* lightSet);

/**
 * @brief Gets the ambient color for the light set.
 * @remark errno will be set on failure.
 * @param lightSet The light set.
 * @return The ambient color or NULL if lightSet is NULL.
 */
DS_SCENELIGHTING_EXPORT const dsColor3f* dsSceneLightSet_getAmbientColor(
	const dsSceneLightSet* lightSet);

/**
 * @brief Sets the ambient color for the light set.
 * @remark errno will be set on failure.
 * @param lightSet The light set.
 * @param color The ambient color.
 * @return False if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT bool dsSceneLightSet_setAmbientColor(dsSceneLightSet* lightSet,
	const dsColor3f* color);

/**
 * @brief Gets the ambient color for the light set.
 * @param lightSet The light set.
 * @return The ambient intensity.
 */
DS_SCENELIGHTING_EXPORT float dsSceneLightSet_getAmbientIntensity(const dsSceneLightSet* lightSet);

/**
 * @brief Sets the ambient intensity for the light set.
 * @remark errno will be set on failure.
 * @param lightSet The light set.
 * @param ambientColor The ambient intensity.
 * @return False if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT bool dsSceneLightSet_setAmbientIntensity(dsSceneLightSet* lightSet,
	float intensity);

/**
 * @brief Gets the ambient factor for the light set.
 * @remark errno will be set on failure.
 * @param[out] outAmbient The output to hold the final combined ambient color and intensity.
 * @param lightSet The light set.
 * @return False if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT bool dsSceneLightSet_getAmbient(dsColor3f* outAmbient,
	const dsSceneLightSet* lightSet);

/**
 * @brief Sets the ambient color and intensity for the light set.
 * @remark errno will be set on failure.
 * @param lightSet The light set.
 * @param color The ambient color.
 * @param intensity The ambient intensity.
 * @return False if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT bool dsSceneLightSet_setAmbient(dsSceneLightSet* lightSet,
	const dsColor3f* color, float intensity);

/**
 * @brief Prepares the light set for operations searching for lights.
 * @remark errno will be set on failure.
 * @param lightSet The light set.
 * @param intensityThreshold The threshold below which the light is considered out of view. This
 *     must be > 0. Use DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD for the default value.
 * @return False if an error occurred.
 */
DS_SCENELIGHTING_EXPORT bool dsSceneLightSet_prepare(dsSceneLightSet* lightSet,
	float intensityThreshold);

/**
 * @brief Gets the intensity threshold used in the last call to dsSceneLightSet_prepare().
 * @param lightSet The light set.
 * @return The intensity threshold.
 */
DS_SCENELIGHTING_EXPORT float dsSceneLightSet_getIntensityThreshold(
	const dsSceneLightSet* lightSet);

/**
 * @brief Finds the brightest lights at a position.
 *
 * The main light will always be assigned to index 0 if it's present and visible.
 *
 * @param[out] outBrightestLights The output to contain the brightest lights.
 * @param outLightCount The maximum number of lights in outBrightestLIghts.
 * @param[out] outHasMainLight Whether or not the main light is in the output.
 * @param lightSet The light set.
 * @param position The position to use to evaluate which lights are brightest.
 * @return The number of lights. If this is less than outLightCount, any remaining lights will
 *     be NULL.
 */
DS_SCENELIGHTING_EXPORT uint32_t dsSceneLightSet_findBrightestLights(
	const dsSceneLight** outBrightestLights, uint32_t outLightCount, bool* outHasMainLight,
	const dsSceneLightSet* lightSet, const dsVector3f* position);

/**
 * @brief Visits each light that's inside a frustum.
 * @param lightSet The light set.
 * @param frustum The frustum to intersect with the lights.
 * @param visitor The function to call for each light. This may be NULL if you only want to know
 *     how many lights intersect.
 * @param userData User data to pass to the visitor function.
 * @return The number of lights that intersect with the frustum.
 */
DS_SCENELIGHTING_EXPORT uint32_t dsSceneLightSet_forEachLightInFrustum(
	const dsSceneLightSet* lightSet, const dsFrustum3f* frustum,
	dsSceneLightVisitFunction visitor, void* userData);

/**
 * @brief Destroys a light set.
 * @param lightSet The light set to destroy.
 */
DS_SCENELIGHTING_EXPORT void dsSceneLightSet_destroy(dsSceneLightSet* lightSet);

#ifdef __cplusplus
}
#endif
