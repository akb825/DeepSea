/*
 * Copyright 2016-2023 Aaron Barany
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
#include <DeepSea/Math/Types.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Includes all of the types for shadows used in the DeepSea/Render library.
 */

/**
 * @brief Maximum number of planes in a shadow cull volume.
 */
#define DS_MAX_SHADOW_CULL_PLANES 12U

/**
 * @brief Maximum number of corners in a shadow cull volume.
 *
 * The worst case considered here is one box embedded in another and tilted so the corners poke
 * through.
 */
#define DS_MAX_SHADOW_CULL_CORNERS 32U

/**
 * @brief Struct holding parameters used for computing a shadow projection matrix.
 * @see ShadowProjection.h
 */
typedef struct dsShadowProjection
{
	/**
	 * @brief Bounding box of the points in the shadow volume in shadow space.
	 */
	dsAlignedBox3f pointBounds;

	/**
	 * @brief Matrix defining the space shadows are computed in light projected space.
	 */
	dsMatrix44f shadowSpace;

	/**
	 * @brief Matrix to transform from world space to shadow space.
	 */
	dsMatrix44f worldToShadowSpace;

	/**
	 * @brief The light projection matrix for spot or point lights.
	 */
	dsMatrix44f lightProjection;

	/**
	 * @brief The sin of the angle between the view direction and light direction.
	 */
	float sinViewLight;

	/**
	 * @brief Whether or not lightProjection is set.
	 */
	bool hasLightProjection;

	/**
	 * @brief Whether or not to force uniform shadows.
	 *
	 * This is typically only used when shadowing a scene with an orthographic projection.
	 */
	bool uniform;

	/**
	 * @brief The options used to create the projection matrix.
	 */
	dsProjectionMatrixOptions projectionOptions;
} dsShadowProjection;

/**
 * @brief Struct defining a corner between three planes in a shadow cull volume.
 * @see ShadowCullVolume.h
 */
typedef struct dsShadowCullCorner
{
	/**
	 * @brief The point for the corner.
	 */
	dsVector3f point;

	/**
	 * @brief Bitmask for the indices of the planes that intersect at the corner.
	 *
	 * There will alwyas be at least 3 bits set.
	 */
	uint32_t planes;
} dsShadowCullCorner;

/**
 * @brief Struct defining a culling volume used in shadow mapping.
 * @see ShadowCullVolume.h
 */
typedef struct dsShadowCullVolume
{
	/**
	 * @brief Planes that define the cull volume.
	 */
	dsPlane3f planes[DS_MAX_SHADOW_CULL_PLANES];

	/**
	 * @brief Corners within the cull volume.
	 */
	dsShadowCullCorner corners[DS_MAX_SHADOW_CULL_CORNERS];

	/**
	 * @brief The number of planes in the cull volume.
	 */
	uint32_t planeCount;

	/**
	 * @brief The number of corners in the cull volume.
	 */
	uint32_t cornerCount;
} dsShadowCullVolume;

#ifdef __cplusplus
}
#endif
