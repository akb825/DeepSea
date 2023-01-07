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
#include <DeepSea/Render/Export.h>
#include <DeepSea/Render/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for initializing and using shadow cull volumes.
 *
 * A shadow cull volume can either be for a directional light or a light projection. The light
 * projection would either be used for a spot light or one portion of a point light.
 *
 * The cull volume not only tests if a shape is inside the volume, but also clamps the points of the
 * shape to the volume for building the shadow projection.
 *
 * @see dsShadowCullVolume
 */

/**
 * @brief Builds a shadow cull volume for a directional light.
 * @remark errno will be set on failure.
 * @param volume The cull volume to initialize.
 * @param viewFrustum The frustum defining the view in world space. This frustum should be
 *     normalized.
 * @param toLight The direction to the light.
 * @return False if the parameters are invalid.
 */
DS_RENDER_EXPORT bool dsShadowCullVolume_buildDirectional(dsShadowCullVolume* volume,
	const dsFrustum3f* viewFrustum, const dsVector3f* toLight);

/**
 * @brief Builds a shadow cull volume for a spot light.
 *
 * This can also be used for a cube face for a point light.
 *
 * @remark errno will be set on failure.
 * @param volume The cull volume to initialize.
 * @param viewFrustum The frustum defining the view in world space. This frustum should be
 *     normalized.
 * @param lightFrustum The frustum defining the light in world space. The near plane will be
 *     ignored since a non-zero near is required to create a well-formed frustum and conceptually
 *     the light starts at a point, This frustum should be normalized.
 * @return False if the parameters are invalid.
 */
DS_RENDER_EXPORT bool dsShadowCullVolume_buildSpot(dsShadowCullVolume* volume,
	const dsFrustum3f* viewFrustum, const dsFrustum3f* lightFrustum);

/**
 * @brief Intersects an aligned box with a shadow cull volume.
 * @param volume The cull volume to intersect.
 * @param box The aligned box to intersect with.
 * @param shadowProj Optional shadow projection to add the results to. If not NULL, and the box
 *     intersects with the volume, the corners will be clamped to the volume and added to the
 *     projection.
 * @param clampToVolume Whether or not to clamp the bounds to the cull volume when adding points to
 *     the shadow projection. This can be error prone in some situations, so it's only recommended
 *     for larger bounds that could expand the shadow projection too much.
 * @return The intersection result. Inside and outside is with respect to the volume. If the box
 *     fully contains the frustum, dsIntersectResult_Intersects will be returned.
 */
DS_RENDER_EXPORT dsIntersectResult dsShadowCullVolume_intersectAlignedBox(
	const dsShadowCullVolume* volume, const dsAlignedBox3f* box, dsShadowProjection* shadowProj,
	bool clampToVolume);

#if DS_HAS_SIMD
/**
 * @brief Intersects an aligned box with a shadow cull volume using SIMD operations.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param volume The cull volume to intersect.
 * @param box The aligned box to intersect with.
 * @param shadowProj Optional shadow projection to add the results to. If not NULL, and the box
 *     intersects with the volume, the corners will be clamped to the volume and added to the
 *     projection.
 * @param clampToVolume Whether or not to clamp the bounds to the cull volume when adding points to
 *     the shadow projection. This can be error prone in some situations, so it's only recommended
 *     for larger bounds that could expand the shadow projection too much.
 * @return The intersection result. Inside and outside is with respect to the volume. If the box
 *     fully contains the frustum, dsIntersectResult_Intersects will be returned.
 */
DS_RENDER_EXPORT dsIntersectResult dsShadowCullVolume_intersectAlignedBoxSIMD(
	const dsShadowCullVolume* volume, const dsAlignedBox3f* box, dsShadowProjection* shadowProj,
	bool clampToVolume);

/**
 * @brief Intersects an aligned box with a shadow cull volume using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_FMA is available.
 * @param volume The cull volume to intersect.
 * @param box The aligned box to intersect with.
 * @param shadowProj Optional shadow projection to add the results to. If not NULL, and the box
 *     intersects with the volume, the corners will be clamped to the volume and added to the
 *     projection.
 * @param clampToVolume Whether or not to clamp the bounds to the cull volume when adding points to
 *     the shadow projection. This can be error prone in some situations, so it's only recommended
 *     for larger bounds that could expand the shadow projection too much.
 * @return The intersection result. Inside and outside is with respect to the volume. If the box
 *     fully contains the frustum, dsIntersectResult_Intersects will be returned.
 */
DS_RENDER_EXPORT dsIntersectResult dsShadowCullVolume_intersectAlignedBoxFMA(
	const dsShadowCullVolume* volume, const dsAlignedBox3f* box, dsShadowProjection* shadowProj,
	bool clampToVolume);
#endif

/**
 * @brief Intersects an oriented box with a shadow cull volume.
 * @param volume The cull volume to intersect.
 * @param box The oriented box to intersect with.
 * @param shadowProj Optional shadow projection to add the results to. If not NULL, and the box
 *     intersects with the volume, the corners will be clamped to the volume and added to the
 *     projection.
 * @param clampToVolume Whether or not to clamp the bounds to the cull volume when adding points to
 *     the shadow projection. This can be error prone in some situations, so it's only recommended
 *     for larger bounds that could expand the shadow projection too much.
 * @return The intersection result. Inside and outside is with respect to the volume. If the box
 *     fully contains the frustum, dsIntersectResult_Intersects will be returned.
 */
DS_RENDER_EXPORT dsIntersectResult dsShadowCullVolume_intersectOrientedBox(
	const dsShadowCullVolume* volume, const dsOrientedBox3f* box, dsShadowProjection* shadowProj,
	bool clampToVolume);

#if DS_HAS_SIMD
/**
 * @brief Intersects an oriented box with a shadow cull volume using SIMD operations.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param volume The cull volume to intersect.
 * @param box The oriented box to intersect with.
 * @param shadowProj Optional shadow projection to add the results to. If not NULL, and the box
 *     intersects with the volume, the corners will be clamped to the volume and added to the
 *     projection.
 * @param clampToVolume Whether or not to clamp the bounds to the cull volume when adding points to
 *     the shadow projection. This can be error prone in some situations, so it's only recommended
 *     for larger bounds that could expand the shadow projection too much.
 * @return The intersection result. Inside and outside is with respect to the volume. If the box
 *     fully contains the frustum, dsIntersectResult_Intersects will be returned.
 */
DS_RENDER_EXPORT dsIntersectResult dsShadowCullVolume_intersectOrientedBoxSIMD(
	const dsShadowCullVolume* volume, const dsOrientedBox3f* box, dsShadowProjection* shadowProj,
	bool clampToVolume);

/**
 * @brief Intersects an oriented box with a shadow cull volume using FMA operations.
 * @remark This can be used when dsSIMDFeatures_FMA is available.
 * @param volume The cull volume to intersect.
 * @param box The oriented box to intersect with.
 * @param shadowProj Optional shadow projection to add the results to. If not NULL, and the box
 *     intersects with the volume, the corners will be clamped to the volume and added to the
 *     projection.
 * @param clampToVolume Whether or not to clamp the bounds to the cull volume when adding points to
 *     the shadow projection. This can be error prone in some situations, so it's only recommended
 *     for larger bounds that could expand the shadow projection too much.
 * @return The intersection result. Inside and outside is with respect to the volume. If the box
 *     fully contains the frustum, dsIntersectResult_Intersects will be returned.
 */
DS_RENDER_EXPORT dsIntersectResult dsShadowCullVolume_intersectOrientedBoxFMA(
	const dsShadowCullVolume* volume, const dsOrientedBox3f* box, dsShadowProjection* shadowProj,
	bool clampToVolume);
#endif

/**
 * @brief Intersects a box in matrix form with a shadow cull volume.
 * @param volume The cull volume to intersect.
 * @param boxMatrix The oriented box to intersect with.
 * @param shadowProj Optional shadow projection to add the results to. If not NULL, and the box
 *     intersects with the volume, the corners will be clamped to the volume and added to the
 *     projection.
 * @param clampToVolume Whether or not to clamp the bounds to the cull volume when adding points to
 *     the shadow projection. This can be error prone in some situations, so it's only recommended
 *     for larger bounds that could expand the shadow projection too much.
 * @return The intersection result. Inside and outside is with respect to the volume. If the box
 *     fully contains the frustum, dsIntersectResult_Intersects will be returned.
 */
DS_RENDER_EXPORT dsIntersectResult dsShadowCullVolume_intersectBoxMatrix(
	const dsShadowCullVolume* volume, const dsMatrix44f* boxMatrix, dsShadowProjection* shadowProj,
	bool clampToVolume);

#if DS_HAS_SIMD
/**
 * @brief Intersects a box in matrix form with a shadow cull volume using SIMD operations.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param volume The cull volume to intersect.
 * @param boxMatrix The oriented box to intersect with.
 * @param shadowProj Optional shadow projection to add the results to. If not NULL, and the box
 *     intersects with the volume, the corners will be clamped to the volume and added to the
 *     projection.
 * @param clampToVolume Whether or not to clamp the bounds to the cull volume when adding points to
 *     the shadow projection. This can be error prone in some situations, so it's only recommended
 *     for larger bounds that could expand the shadow projection too much.
 * @return The intersection result. Inside and outside is with respect to the volume. If the box
 *     fully contains the frustum, dsIntersectResult_Intersects will be returned.
 */
DS_RENDER_EXPORT dsIntersectResult dsShadowCullVolume_intersectBoxMatrixSIMD(
	const dsShadowCullVolume* volume, const dsMatrix44f* boxMatrix, dsShadowProjection* shadowProj,
	bool clampToVolume);

/**
 * @brief Intersects a box in matrix form with a shadow cull volume using FMA operations.
 * @remark This can be used when dsSIMDFeatures_FMA is available.
 * @param volume The cull volume to intersect.
 * @param boxMatrix The oriented box to intersect with.
 * @param shadowProj Optional shadow projection to add the results to. If not NULL, and the box
 *     intersects with the volume, the corners will be clamped to the volume and added to the
 *     projection.
 * @param clampToVolume Whether or not to clamp the bounds to the cull volume when adding points to
 *     the shadow projection. This can be error prone in some situations, so it's only recommended
 *     for larger bounds that could expand the shadow projection too much.
 * @return The intersection result. Inside and outside is with respect to the volume. If the box
 *     fully contains the frustum, dsIntersectResult_Intersects will be returned.
 */
DS_RENDER_EXPORT dsIntersectResult dsShadowCullVolume_intersectBoxMatrixFMA(
	const dsShadowCullVolume* volume, const dsMatrix44f* boxMatrix, dsShadowProjection* shadowProj,
	bool clampToVolume);
#endif

/**
 * @brief Intersects a sphere a shadow cull volume.
 * @param volume The cull volume to intersect.
 * @param center The center of the sphere.
 * @param radius The radius of the sphere.
 * @param shadowProj Optional shadow projection to add the results to. If not NULL, and the sphere
 *     intersects with the volume, the corners of a box fitting the sphere will be clamped to the
 *     volume and added to the projection.
 * @param clampToVolume Whether or not to clamp the bounds to the cull volume when adding points to
 *     the shadow projection. This can be error prone in some situations, so it's only recommended
 *     for larger bounds that could expand the shadow projection too much.
 * @return The intersection result. Inside and outside is with respect to the volume. If the sphere
 *     fully contains the frustum, dsIntersectResult_Intersects will be returned.
 */
DS_RENDER_EXPORT dsIntersectResult dsShadowCullVolume_intersectSphere(
	const dsShadowCullVolume* volume, const dsVector3f* center, float radius,
	dsShadowProjection* shadowProj, bool clampToVolume);

#ifdef __cplusplus
}
#endif
