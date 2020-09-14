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
#include <DeepSea/Geometry/Types.h>
#include <DeepSea/SceneLighting/Export.h>
#include <DeepSea/SceneLighting/Types.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief The default intensity threshold below which a light is considered out of view.
 */
#define DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD 0.1f

/**
 * @file
 * @brief Functions for manipulating scene lights.
 * @see dsSceneLight
 */

/**
 * @brief Makes a directional light.
 * @remark errno will be set on failure.
 * @param[out] outLight The light to to set.
 * @param direction The direction of the light. This should be normalized.
 * @param color The color of the light.
 * @param intensity The intensity of the light.
 * @return False if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT bool dsSceneLight_makeDirectional(dsSceneLight* outLight,
	const dsVector3f* direction, const dsColor3f* color, float intensity);

/**
 * @brief Makes a point light.
 * @remark errno will be set on failure.
 * @param[out] outLight The light to to set.
 * @param position The position of the light.
 * @param color The color of the light.
 * @param intensity The intensity of the light.
 * @param linearFalloff The linear falloff factor for the light.
 * @param quadraticFalloff The quadratic falloff factor for the light.
 * @return False if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT bool dsSceneLight_makePoint(dsSceneLight* outLight,
	const dsVector3f* position, const dsColor3f* color, float intensity, float linearFalloff,
	float quadraticFalloff);

/**
 * @brief Makes a spot light.
 * @remark errno will be set on failure.
 * @param[out] outLight The light to to set.
 * @param position The position of the light.
 * @param direction The direction of the light. This should be normalized.
 * @param color The color of the light.
 * @param intensity The intensity of the light.
 * @param linearFalloff The linear falloff factor for the light.
 * @param quadraticFalloff The quadratic falloff factor for the light.
 * @param innerSpotCosAngle The cosine of the angle from the direction that the light starts to cut
 *     off.
 * @param outerSpotCosAngle The cosine of the angle from the direction that the light is fully cut
 *     off.
 * @return False if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT bool dsSceneLight_makeSpot(dsSceneLight* outLight,
	const dsVector3f* position, const dsVector3f* direction, const dsColor3f* color,
	float intensity, float linearFalloff, float quadraticFalloff, float innerSpotCosAngle,
	float outerSpotCosAngle);

/**
 * @brief Gets the falloff multiplier based on a position.
 * @param light The light to get the falloff multiplier for.
 * @param position The reference position.
 * @return The value to multiply the color and intensity for the falloff.
 */
DS_SCENELIGHTING_EXPORT float dsSceneLight_getFalloff(const dsSceneLight* light,
	const dsVector3f* position);

/**
 * @brief Gets the final intensity of the light based on a position.
 *
 * This takes the grayscale value of the color and the intensity of the light into consideration.
 *
 * @param light The light to get the intensity for.
 * @param position The reference position.
 * @return The intensity at the position.
 */
DS_SCENELIGHTING_EXPORT float dsSceneLight_getIntensity(const dsSceneLight* light,
	const dsVector3f* position);

/**
 * @brief Computes the bounding box for a light.
 * @remark errno will be set on failure.
 * @param[out] outBounds The aligned bounding box for the light.
 * @param light The light to compute the bounding box for.
 * @param intensityThreshold The threshold below which the light is considered out of view. This
 *     must be > 0. Use DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD for the default value.
 * @return False if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT bool dsSceneLight_computeBounds(dsAlignedBox3f* outBounds,
	const dsSceneLight* light, float intensityThreshold);

#ifdef __cplusplus
}
#endif
