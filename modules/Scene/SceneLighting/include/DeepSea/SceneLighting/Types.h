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
#include <DeepSea/Math/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Includes all of the types used in the DeepSea/SceneLighting library.
 */

/**
 * @brief Log tag used by the scene lighting library.
 */
#define DS_SCENE_LIGHTING_LOG_TAG "scene-lighting"

/**
 * @brief Enum for the type of a light.
 */
typedef enum dsSceneLightType
{
	dsSceneLightType_Directional, ///< Infinite light shining in a direction.
	dsSceneLightType_Point,       ///< Light shining in all directions from a point.
	dsSceneLightType_Spot         ///< Light shining in a direction from a point.
} dsSceneLightType;

/**
 * @brief Struct defining a light within a scene.
 *
 * Depending on the parameters, this can represent a directional, point, or spot light.
 *
 * @see SceneLight.h
 */
typedef struct dsSceneLight
{
	/**
	 * @brief The type of the light.
	 */
	dsSceneLightType type;

	/**
	 * @brief The position of the light.
	 */
	dsVector3f position;

	/**
	 * @brief The direction of the light.
	 */
	dsVector3f direction;

	/**
	 * @brief The color of the light.
	 *
	 * When doing sRGB correct rendering, this light should be in linear space.
	 */
	dsColor3f color;

	/**
	 * @brief The intensity of the light.
	 *
	 * This will multiply with the color for the final color to draw with. This allows the
	 * brightness and color to be adjusted independently more easily.
	 */
	float intensity;

	/**
	 * @brief Linear falloff factor for point and spot lights.
	 */
	float linearFalloff;

	/**
	 * @brief Quadratic falloff factor for point and spot lights.
	 */
	float quadraticFalloff;

	/**
	 * @brief The cosine of the inner angle for a spot.
	 *
	 * This is the angle when a spot light starts to fade out.
	 */
	float innerSpotCosAngle;

	/**
	 * @brief The cosine of the outer angle for a spot.
	 *
	 * This is the angle when a spot light finishes fading out.
	 */
	float outerSpotCosAngle;
} dsSceneLight;

/**
 * Struct defining the set of lights used in a scene.
 */
typedef struct dsSceneLightSet dsSceneLightSet;

#ifdef __cplusplus
}
#endif
