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
#include <DeepSea/Render/Resources/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions to create and manipulate shadow managers for the scene.
 * @see dsSceneShadowManager
 */

/**
 * @brief Creates scene light shadows to manage shadows for a single light.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create this with. This must support freeing memory.
 * @param resourceManager The resource manager to create graphics resources with.
 * @param lightSet The light set to retrieve lights from.
 * @param lightType The type of the light this will cast shadows for.
 * @param lightName The name of the light to get shadows for. This may be NULL if it will be set
 *     later.
 * @param matrixGroupDesc The shader variable group that contains the matrices. This must have a
 *     elements based on lightType:
 *     - Directional:
 *         - mat44 array for the shadow projection of size 4, or non-array element if not cascaded.
 *         - vec4 of floats for cascade split distances, or omitted if not cascaded.
 *         - vec2 for the distance to start fading out shadows and maximum shadow distance.
 *     - Point:
 *         - mat44 array of 6 elements for the shadow projection.
 *         - vec2 for the distance to start fading out shadows and maximum shadow distance.
 *     - Spot:
 *         - mat44 for the shadow projection.
 *         - vec2 for the distance to start fading out shadows and maximum shadow distance.
 * @param shadowParams Parameters controlling the shadow behavior.
 * @return The light shadows or NULL if an error occurred.
 */
DS_SCENELIGHTING_EXPORT dsSceneLightShadows* dsSceneLightShadows_create(dsAllocator* allocator,
	dsResourceManager* resourceManager, const dsSceneLightSet* lightSet, dsSceneLightType lightType,
	const char* lightName, const dsShaderVariableGroupDesc* matrixGroupDesc,
	const dsSceneShadowParams* shadowParams);

/**
 * @brief Gets the light type that will be shadowed.
 * @param shadows The scene light shadows.
 * @return The light type.
 */
DS_SCENELIGHTING_EXPORT dsSceneLightType dsSceneLightShadows_getLightType(
	const dsSceneLightShadows* shadows);

/**
 * @brief Gets the ID for the light being shadowed.
 * @param shadows The scene light shadows.
 * @return The light ID.
 */
DS_SCENELIGHTING_EXPORT uint32_t dsSceneLightShadows_getLightID(const dsSceneLightShadows* shadows);

/**
 * @brief Sets the ID for the light being shadowed.
 * @remark errno will be set on failure.
 * @param shadows The scene light shadows.
 * @param lightID The light ID.
 * @return False if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT bool dsSceneLightShadows_setLightID(dsSceneLightShadows* shadows,
	uint32_t lightID);

/**
 * @brief Sets the name for the light being shadowed.
 * @remark errno will be set on failure.
 * @param shadows The scene light shadows.
 * @param light The name of the light. This may be NULL to disable the shadows.
 * @return False if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT bool dsSceneLightShadows_setLightName(dsSceneLightShadows* shadows,
	const char* light);

/**
 * @brief Gets the maximum cascades for shadows.
 * @param shadows The scene light shadows.
 * @return The maximum number of cascades, or 0 if the shadows aren't cascaded.
 */
DS_SCENELIGHTING_EXPORT uint32_t dsSceneLightShadows_getMaxCascades(
	const dsSceneLightShadows* shadows);

/**
 * @brief Sets the maximum cascades for shadows.
 * @remark errno will be set on failure.
 * @param shadows The scene light shadows.
 * @param maxCascades The maximum number of cascades, or 0 if the shadows aren't cascaded.
 * @return False if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT bool dsSceneLightShadows_setMaxCascades(dsSceneLightShadows* shadows,
	uint32_t maxCascades);

/**
 * @brief Gets the maximum distance for the first split for cascaded shadows.
 * @param shadows The scene light shadows.
 * @return The maximum distance for the first split.
 */
DS_SCENELIGHTING_EXPORT float dsSceneLightShadows_getMaxFirstSplitDistance(
	const dsSceneLightShadows* shadows);

/**
 * @brief Sets the maximum distance for the first split for cascaded shadows.
 * @remark errno will be set on failure.
 * @param shadows The scene light shadows.
 * @param maxDistance The maximum distance for the first split.
 * @return False if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT bool dsSceneLightShadows_setMaxFirstSplitDistance(
	dsSceneLightShadows* shadows, float maxDistance);

/**
 * @brief Gets the exponential factor for cascaded shadows.
 * @param shadows The scene light shadows.
 * @return The exponential factor for cascaded shadows.
 */
DS_SCENELIGHTING_EXPORT float dsSceneLightShadows_getCascadedExpFactor(
	const dsSceneLightShadows* shadows);

/**
 * @brief Sets the exponential factor for cascaded shadows.
 * @remark errno will be set on failure.
 * @param shadows The scene light shadows.
 * @param expFactor The exponential factor for cascaded shadows in the range [0, 1], where 0 uses
 *     linear distances between the splits and 1 is fully exponential.
 * @return False if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT bool dsSceneLightShadows_setCascadedExpFactor(
	dsSceneLightShadows* shadows, float expFactor);

/**
 * @brief Gets the distance to start fading out shadows.
 * @param shadows The scene light shadows.
 * @return The distance to start fading out shadows.
 */
DS_SCENELIGHTING_EXPORT float dsSceneLightShadows_getFadeStartDistance(
	const dsSceneLightShadows* shadows);

/**
 * @brief Sets the distance to start fading out shadows.
 * @remark errno will be set on failure.
 * @param shadows The scene light shadows.
 * @param distance The distance to start fading out shadows.
 * @return False if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT bool dsSceneLightShadows_setFadeStartDistance(
	dsSceneLightShadows* shadows, float distance);

/**
 * @brief Gets the maximum distance to display shadows.
 * @param shadows The scene light shadows.
 * @return The maximum shadow distance.
 */
DS_SCENELIGHTING_EXPORT float dsSceneLightShadows_getMaxDistance(
	const dsSceneLightShadows* shadows);

/**
 * @brief Sets the maximum distance to display shadows.
 * @remark errno will be set on failure.
 * @param shadows The scene light shadows.
 * @param distance The maximum shadow distance.
 * @return False if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT bool dsSceneLightShadows_setMaxDistance(
	dsSceneLightShadows* shadows, float distance);

/**
 * @brief Prepares the scene light shadows for the next frame.
 * @remark errno will be set on failure.
 * @param shadows The scene light shadows to prepare.
 * @param view The view to prepare for.
 * @return False if an error occurred.
 */
DS_SCENELIGHTING_EXPORT bool dsSceneLightShadows_prepare(dsSceneLightShadows* shadows,
	const dsView* view);

/**
 * @brief Gets the number of shadow surfaces to draw this frame.
 * @param shadows The scene light shadows.
 * @return The number of surfaces. This will be 0 if the shadows aren't visible.
 */
DS_SCENELIGHTING_EXPORT uint32_t dsSceneLightShadows_getSurfaceCount(
	const dsSceneLightShadows* shadows);

/**
 * @brief Intersects an aligned box with a shadow surface.
 * @param shadows The scene light shadows.
 * @param surface The shadow surface index.
 * @param box The aligned box to intersect with.
 * @return The intersection result. Inside and outside is with respect to the volume. If the box
 *     fully contains the frustum, dsIntersectResult_Intersects will be returned.
 */
DS_SCENELIGHTING_EXPORT dsIntersectResult dsSceneLightShadows_intersectAlignedBox(
	dsSceneLightShadows* shadows, uint32_t surface, const dsAlignedBox3f* box);

/**
 * @brief Intersects an oriented box with a shadow surface.
 * @param shadows The scene light shadows.
 * @param surface The shadow surface index.
 * @param box The oriented box to intersect with.
 * @return The intersection result. Inside and outside is with respect to the volume. If the box
 *     fully contains the frustum, dsIntersectResult_Intersects will be returned.
 */
DS_SCENELIGHTING_EXPORT dsIntersectResult dsSceneLightShadows_intersectOrientedBox(
	dsSceneLightShadows* shadows, uint32_t surface, const dsOrientedBox3f* box);

/**
 * @brief Intersects a sphere a shadow surface.
 * @param shadows The scene light shadows.
 * @param surface The shadow surface index.
 * @param center The center of the sphere.
 * @param radius The radius of the sphere.
 * @return The intersection result. Inside and outside is with respect to the volume. If the sphere
 *     fully contains the frustum, dsIntersectResult_Intersects will be returned.
 */
DS_SCENELIGHTING_EXPORT dsIntersectResult dsSceneLightShadows_intersectSphere(
	dsSceneLightShadows* shadows, uint32_t surface, const dsVector3f* center, float radius);

/**
 * @brief Computes the projection of a shadow surface.
 * @remark errno will be set on failure.
 * @param shadows The scene light shadows.
 * @param surface The surface index.
 * @return False if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT bool dsSceneLightShadows_computeSurfaceProjection(
	dsSceneLightShadows* shadows, uint32_t surface);

/**
 * @brief Destroys a scene light shadows instance.
 * @remark errno will be set on failure.
 * @param shadows The scene light shadows to destroy.
 * @return False if the graphics resources couldn't be deleted.
 */
DS_SCENELIGHTING_EXPORT bool dsSceneLightShadows_destroy(dsSceneLightShadows* shadows);

#ifdef __cplusplus
}
#endif

