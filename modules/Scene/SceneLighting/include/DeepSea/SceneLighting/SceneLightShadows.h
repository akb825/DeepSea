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
 * @brief Functions to create and manipulate scene light shadows.
 * @see dsSceneLightShadows
 */

/**
 * @brief The maximum number of surfaces that can be in a scene light shadows instance.
 */
#define DS_MAX_SCENE_LIGHT_SHADOWS_SURFACES 6

/**
 * @brief Creates scene light shadows to manage shadows for a single light.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create this with. This must support freeing memory.
 * @param name The name of the light shadows. This will be copied. When used with deferred lighting,
 *     this is expected to have the same name as the texture in the view.
 * @param resourceManager The resource manager to create graphics resources with.
 * @param lightSet The light set to retrieve the light from.
 * @param lightType The type of the light this will cast shadows for.
 * @param lightName The name of the light to get shadows for. This may be NULL if it will be set
 *     later.
 * @param transformGroupDesc The shader variable group that contains the shadow transform. Transform
 *     matrices go from view space to shadow projection space. This must have a elements based on
 *     lightType:
 *     - Directional:
 *         - mat44 array for the shadow projection of size 4, or non-array element if not cascaded.
 *         - vec4 of floats for cascade split distances, or omitted if not cascaded.
 *         - vec2 for the distance to start fading out shadows and maximum shadow distance.
 *     - Point:
 *         - mat44 array of 6 elements for the shadow projection.
 *         - vec2 for the distance to start fading out shadows and maximum shadow distance.
 *         - vec3 for the position of the light in view space.
 *     - Spot:
 *         - mat44 for the shadow projection.
 *         - vec2 for the distance to start fading out shadows and maximum shadow distance.
 * @param transformGroupName The name of the transform group when set on the view's global
 *     variables. This may be NULL when only set as an instance variable.
 * @param shadowParams Parameters controlling the shadow behavior.
 * @return The light shadows or NULL if an error occurred.
 */
DS_SCENELIGHTING_EXPORT dsSceneLightShadows* dsSceneLightShadows_create(dsAllocator* allocator,
	const char* name, dsResourceManager* resourceManager, const dsSceneLightSet* lightSet,
	dsSceneLightType lightType, const char* lightName,
	const dsShaderVariableGroupDesc* transformGroupDesc, const char* transformGroupName,
	const dsSceneShadowParams* shadowParams);

/**
 * @brief Gets the name of the scene light shadows.
 * @remark errno will be set on failure.
 * @param shadows The scene light shadows.
 * @return The name or NULL if shadows is NULL.
 */
DS_SCENELIGHTING_EXPORT const char* dsSceneLightShadows_getName(const dsSceneLightShadows* shadows);

/**
 * @brief Gets the name ID of the scene light shadows.
 * @remark errno will be set on failure.
 * @param shadows The scene light shadows.
 * @return The name ID.
 */
DS_SCENELIGHTING_EXPORT uint32_t dsSceneLightShadows_getNameID(const dsSceneLightShadows* shadows);

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
 * @brief Getds the minimum depth ranges for each cascade.
 * @remark errno will be set on failure.
 * @param shadows The scene light shadows.
 * @return The depth ranges for each cascade or NULL if shadows is NULL.
 */
DS_SCENELIGHTING_EXPORT const float* dsSceneLightShadows_getMinDepthRanges(
	const dsSceneLightShadows* shadows);

/**
 * @brief Sets the minimum depth ranges for a range of cascades.
 *
 * Spot and point light shadows will only use the first cascade's value.
 *
 * @remark errno will be set on failure.
 * @param shadows The scene light shadows.
 * @param depthRanges A pointer to the depth ranges.
 * @param start The first cascade index to assign.
 * @param count The number of cascades to assign.
 * @return False if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT bool dsSceneLightShadows_setMinDepthRanges(dsSceneLightShadows* shadows,
	const float* depthRanges, uint32_t start, uint32_t count);

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
 * @brief Binds the transform shader variable group.
 * @remark errno will be set on failure.
 * @param shadows The scenen light shadows.
 * @param materialValues The material values to bind to.
 * @param nameID The ID for the name to bind to.
 * @return False if the values couldn't be bound.
 */
DS_SCENELIGHTING_EXPORT bool dsSceneLightShadows_bindTransformGroup(
	const dsSceneLightShadows* shadows, dsSharedMaterialValues* materialValues, uint32_t nameID);

/**
 * @brief Gets the number of shadow surfaces to draw this frame.
 * @param shadows The scene light shadows.
 * @return The number of surfaces. This will be 0 if the shadows aren't visible.
 */
DS_SCENELIGHTING_EXPORT uint32_t dsSceneLightShadows_getSurfaceCount(
	const dsSceneLightShadows* shadows);

/**
 * @brief Intersects an aligned box in world space with a shadow surface.
 * @param shadows The scene light shadows.
 * @param surface The shadow surface index.
 * @param box The aligned box to intersect with in world space.
 * @return The intersection result. Inside and outside is with respect to the volume. If the box
 *     fully contains the frustum, dsIntersectResult_Intersects will be returned.
 */
DS_SCENELIGHTING_EXPORT dsIntersectResult dsSceneLightShadows_intersectAlignedBox(
	dsSceneLightShadows* shadows, uint32_t surface, const dsAlignedBox3f* box);

#if DS_HAS_SIMD
/**
 * @brief Intersects an aligned box in world space with a shadow surface using SIMD operations
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param shadows The scene light shadows.
 * @param surface The shadow surface index.
 * @param box The aligned box to intersect with in world space.
 * @return The intersection result. Inside and outside is with respect to the volume. If the box
 *     fully contains the frustum, dsIntersectResult_Intersects will be returned.
 */
DS_SCENELIGHTING_EXPORT dsIntersectResult dsSceneLightShadows_intersectAlignedBoxSIMD(
	dsSceneLightShadows* shadows, uint32_t surface, const dsAlignedBox3f* box);

/**
 * @brief Intersects an aligned box in world space with a shadow surface using fused multiply-add
 *     operations
 * @remark This can be used when dsSIMDFeatures_FMA is available.
 * @param shadows The scene light shadows.
 * @param surface The shadow surface index.
 * @param box The aligned box to intersect with in world space.
 * @return The intersection result. Inside and outside is with respect to the volume. If the box
 *     fully contains the frustum, dsIntersectResult_Intersects will be returned.
 */
DS_SCENELIGHTING_EXPORT dsIntersectResult dsSceneLightShadows_intersectAlignedBoxFMA(
	dsSceneLightShadows* shadows, uint32_t surface, const dsAlignedBox3f* box);
#endif

/**
 * @brief Intersects a box in matrix form in world space with a shadow surface.
 * @param shadows The scene light shadows.
 * @param surface The shadow surface index.
 * @param boxMatrix The box in matrix form to intersect with in world space.
 * @return The intersection result. Inside and outside is with respect to the volume. If the box
 *     fully contains the frustum, dsIntersectResult_Intersects will be returned.
 */
DS_SCENELIGHTING_EXPORT dsIntersectResult dsSceneLightShadows_intersectBoxMatrix(
	dsSceneLightShadows* shadows, uint32_t surface, const dsMatrix44f* boxMatrix);

#if DS_HAS_SIMD
/**
 * @brief Intersects a box in matrix form in world space with a shadow surface using SIMD
 *     operations.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param shadows The scene light shadows.
 * @param surface The shadow surface index.
 * @param boxMatrix The box in matrix form to intersect with in world space.
 * @return The intersection result. Inside and outside is with respect to the volume. If the box
 *     fully contains the frustum, dsIntersectResult_Intersects will be returned.
 */
DS_SCENELIGHTING_EXPORT dsIntersectResult dsSceneLightShadows_intersectBoxMatrixSIMD(
	dsSceneLightShadows* shadows, uint32_t surface, const dsMatrix44f* boxMatrix);

/**
 * @brief Intersects a box in matrix form in world space with a shadow surface using fused
 *     multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_FMA is available.
 * @param shadows The scene light shadows.
 * @param surface The shadow surface index.
 * @param boxMatrix The box in matrix form to intersect with in world space.
 * @return The intersection result. Inside and outside is with respect to the volume. If the box
 *     fully contains the frustum, dsIntersectResult_Intersects will be returned.
 */
DS_SCENELIGHTING_EXPORT dsIntersectResult dsSceneLightShadows_intersectBoxMatrixFMA(
	dsSceneLightShadows* shadows, uint32_t surface, const dsMatrix44f* boxMatrix);
#endif

/**
 * @brief Intersects an oriented box in world space with a shadow surface.
 * @param shadows The scene light shadows.
 * @param surface The shadow surface index.
 * @param box The oriented box to intersect with in world space.
 * @return The intersection result. Inside and outside is with respect to the volume. If the box
 *     fully contains the frustum, dsIntersectResult_Intersects will be returned.
 */
DS_SCENELIGHTING_EXPORT dsIntersectResult dsSceneLightShadows_intersectOrientedBox(
	dsSceneLightShadows* shadows, uint32_t surface, const dsOrientedBox3f* box);

#if DS_HAS_SIMD
/**
 * @brief Intersects an oriented box in world space with a shadow surface using SIMD operations.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param shadows The scene light shadows.
 * @param surface The shadow surface index.
 * @param box The oriented box to intersect with in world space.
 * @return The intersection result. Inside and outside is with respect to the volume. If the box
 *     fully contains the frustum, dsIntersectResult_Intersects will be returned.
 */
DS_SCENELIGHTING_EXPORT dsIntersectResult dsSceneLightShadows_intersectOrientedBoxSIMD(
	dsSceneLightShadows* shadows, uint32_t surface, const dsOrientedBox3f* box);

/**
 * @brief Intersects an oriented box in world space with a shadow surface using fused multiply-add
 *     operations.
 * @remark This can be used when dsSIMDFeatures_FMA is available.
 * @param shadows The scene light shadows.
 * @param surface The shadow surface index.
 * @param box The oriented box to intersect with in world space.
 * @return The intersection result. Inside and outside is with respect to the volume. If the box
 *     fully contains the frustum, dsIntersectResult_Intersects will be returned.
 */
DS_SCENELIGHTING_EXPORT dsIntersectResult dsSceneLightShadows_intersectOrientedBoxFMA(
	dsSceneLightShadows* shadows, uint32_t surface, const dsOrientedBox3f* box);
#endif

/**
 * @brief Intersects a sphere in world space with a shadow surface.
 * @param shadows The scene light shadows.
 * @param surface The shadow surface index.
 * @param center The center of the sphere in world space.
 * @param radius The radius of the sphere.
 * @return The intersection result. Inside and outside is with respect to the volume. If the sphere
 *     fully contains the frustum, dsIntersectResult_Intersects will be returned.
 */
DS_SCENELIGHTING_EXPORT dsIntersectResult dsSceneLightShadows_intersectSphere(
	dsSceneLightShadows* shadows, uint32_t surface, const dsVector3f* center, float radius);

/**
 * @brief Intersects an aligned box in view space with a shadow surface.
 * @param shadows The scene light shadows.
 * @param surface The shadow surface index.
 * @param box The aligned box to intersect with in view space.
 * @return The intersection result. Inside and outside is with respect to the volume. If the box
 *     fully contains the frustum, dsIntersectResult_Intersects will be returned.
 */
DS_SCENELIGHTING_EXPORT dsIntersectResult dsSceneLightShadows_intersectViewAlignedBox(
	dsSceneLightShadows* shadows, uint32_t surface, const dsAlignedBox3f* box);

#if DS_HAS_SIMD
/**
 * @brief Intersects an aligned box in view space with a shadow surface using SIMD operations.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param shadows The scene light shadows.
 * @param surface The shadow surface index.
 * @param box The aligned box to intersect with in view space.
 * @return The intersection result. Inside and outside is with respect to the volume. If the box
 *     fully contains the frustum, dsIntersectResult_Intersects will be returned.
 */
DS_SCENELIGHTING_EXPORT dsIntersectResult dsSceneLightShadows_intersectViewAlignedBoxSIMD(
	dsSceneLightShadows* shadows, uint32_t surface, const dsAlignedBox3f* box);

/**
 * @brief Intersects an aligned box in view space with a shadow surface using fused multiply-add
 *     operations.
 * @remark This can be used when dsSIMDFeatures_FMA is available.
 * @param shadows The scene light shadows.
 * @param surface The shadow surface index.
 * @param box The aligned box to intersect with in view space.
 * @return The intersection result. Inside and outside is with respect to the volume. If the box
 *     fully contains the frustum, dsIntersectResult_Intersects will be returned.
 */
DS_SCENELIGHTING_EXPORT dsIntersectResult dsSceneLightShadows_intersectViewAlignedBoxFMA(
	dsSceneLightShadows* shadows, uint32_t surface, const dsAlignedBox3f* box);
#endif

/**
 * @brief Intersects an oriented box in view space with a shadow surface.
 * @param shadows The scene light shadows.
 * @param surface The shadow surface index.
 * @param box The oriented box to intersect with in view space.
 * @return The intersection result. Inside and outside is with respect to the volume. If the box
 *     fully contains the frustum, dsIntersectResult_Intersects will be returned.
 */
DS_SCENELIGHTING_EXPORT dsIntersectResult dsSceneLightShadows_intersectViewOrientedBox(
	dsSceneLightShadows* shadows, uint32_t surface, const dsOrientedBox3f* box);

#if DS_HAS_SIMD
/**
 * @brief Intersects an oriented box in view space with a shadow surface using SIMD operations.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param shadows The scene light shadows.
 * @param surface The shadow surface index.
 * @param box The oriented box to intersect with in view space.
 * @return The intersection result. Inside and outside is with respect to the volume. If the box
 *     fully contains the frustum, dsIntersectResult_Intersects will be returned.
 */
DS_SCENELIGHTING_EXPORT dsIntersectResult dsSceneLightShadows_intersectViewOrientedBoxSIMD(
	dsSceneLightShadows* shadows, uint32_t surface, const dsOrientedBox3f* box);

/**
 * @brief Intersects an oriented box in view space with a shadow surface using fused multiply-add
 *     operations.
 * @remark This can be used when dsSIMDFeatures_FMA is available.
 * @param shadows The scene light shadows.
 * @param surface The shadow surface index.
 * @param box The oriented box to intersect with in view space.
 * @return The intersection result. Inside and outside is with respect to the volume. If the box
 *     fully contains the frustum, dsIntersectResult_Intersects will be returned.
 */
DS_SCENELIGHTING_EXPORT dsIntersectResult dsSceneLightShadows_intersectViewOrientedBoxFMA(
	dsSceneLightShadows* shadows, uint32_t surface, const dsOrientedBox3f* box);
#endif

/**
 * @brief Intersects a box in matrix form in view space with a shadow surface.
 * @param shadows The scene light shadows.
 * @param surface The shadow surface index.
 * @param boxMatrix The box in matrix form to intersect with in view space.
 * @return The intersection result. Inside and outside is with respect to the volume. If the box
 *     fully contains the frustum, dsIntersectResult_Intersects will be returned.
 */
DS_SCENELIGHTING_EXPORT dsIntersectResult dsSceneLightShadows_intersectViewBoxMatrix(
	dsSceneLightShadows* shadows, uint32_t surface, const dsMatrix44f* boxMatrix);

#if DS_HAS_SIMD
/**
 * @brief Intersects a box in matrix form in view space with a shadow surface using SIMD operations.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param shadows The scene light shadows.
 * @param surface The shadow surface index.
 * @param boxMatrix The box in matrix form to intersect with in view space.
 * @return The intersection result. Inside and outside is with respect to the volume. If the box
 *     fully contains the frustum, dsIntersectResult_Intersects will be returned.
 */
DS_SCENELIGHTING_EXPORT dsIntersectResult dsSceneLightShadows_intersectViewBoxMatrixSIMD(
	dsSceneLightShadows* shadows, uint32_t surface, const dsMatrix44f* boxMatrix);

/**
 * @brief Intersects a box in matrix form in view space with a shadow surface using fused
 *     multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_FMA is available.
 * @param shadows The scene light shadows.
 * @param surface The shadow surface index.
 * @param boxMatrix The box in matrix form to intersect with in view space.
 * @return The intersection result. Inside and outside is with respect to the volume. If the box
 *     fully contains the frustum, dsIntersectResult_Intersects will be returned.
 */
DS_SCENELIGHTING_EXPORT dsIntersectResult dsSceneLightShadows_intersectViewBoxMatrixFMA(
	dsSceneLightShadows* shadows, uint32_t surface, const dsMatrix44f* boxMatrix);
#endif

/**
 * @brief Intersects a sphere in view space with a shadow surface.
 * @param shadows The scene light shadows.
 * @param surface The shadow surface index.
 * @param center The center of the sphere in view space.
 * @param radius The radius of the sphere.
 * @return The intersection result. Inside and outside is with respect to the volume. If the sphere
 *     fully contains the frustum, dsIntersectResult_Intersects will be returned.
 */
DS_SCENELIGHTING_EXPORT dsIntersectResult dsSceneLightShadows_intersectViewSphere(
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
 * @brief Gets the shadow projection matrix for a surface.
 * @remark errno will be set on failure.
 * @param shadows The scene light shadows.
 * @param surface The surface index.
 * @return The shadow projection matrix or NULL if the parameters are invalid or the projection
 *     hasn't been computed.
 */
DS_SCENELIGHTING_EXPORT const dsMatrix44f* dsSceneLightShadows_getSurfaceProjection(
	const dsSceneLightShadows* shadows, uint32_t surface);

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
