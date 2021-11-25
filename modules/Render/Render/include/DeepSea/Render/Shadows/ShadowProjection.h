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
 * @brief Functions to compute a shadow projection matrix.
 *
 * After initializing a shadow projection, points are added (generally based on bounding volumes
 * clamped to the cull volume) to the projection. Once all the points for the scene are added, the
 * final projection matrix can be computed.
 *
 * @see dsShadowProjection
 */

/**
 * @brief Initializes a shadow projection with the shadow space information.
 * @remark errno will be set on failure.
 * @param shadowProj The shadow projection to initialize.
 * @param renderer The renderer the shadow projection will be used with.
 * @param camera The camera matrix defining the view position and orientation. It's assumed this is
  *    orthonormal.
 * @param toLight The direction to the light. For a point or spot light, this should be the
 *     direction at the center of the volume. This is assumed to be normalized.
 * @param lightTransform The transform matrix for a spot or point light. In the case of a point
 *     light, it will typically be used for one face of a cubemap. This should be NULL for
 *     directional lights.
 * @param lightProjection The projection matrix for a spot or point light. This should be NULL for
 *     directional lights.
 * @param uniform True to force uniform shadows rather than light-space perspective. This should
 *     typically be false except when rendering with an orthographic projection.
 * @return False if the parameters are invalid.
 */
DS_RENDER_EXPORT bool dsShadowProjection_initialize(dsShadowProjection* shadowProj,
	const dsRenderer* renderer, const dsMatrix44f* camera, const dsVector3f* toLight,
	const dsMatrix44f* lightTransform, const dsMatrix44f* lightProjection, bool uniform);

/**
 * @brief Resets a shadow projection to re-compute based on a new set of points.
 * @remark errno will be set on failure.
 * @param shadowProj The shadow projection.
 * @return False if shadowProj is NULL.
 */
DS_RENDER_EXPORT bool dsShadowProjection_reset(dsShadowProjection* shadowProj);

/**
 * @brief Adds points to the shadow projection.
 *
 * The points are typically from a bounding volume clamped to the cull volume.
 *
 * @remark errno will be set on failure.
 * @param shadowProj The shadow projection to add the points to.
 * @param points The list of points in world space to add.
 * @param pointCount The number of points.
 * @return False if the parameters are invalid.
 */
DS_RENDER_EXPORT bool dsShadowProjection_addPoints(dsShadowProjection* shadowProj,
	const dsVector3f* points, uint32_t pointCount);

/**
 * @brief Computes the projection matrix for a shadow projection.
 * @param[out] outMatrix The projection matrix for the shadow projection.
 * @param shadowProj The shadow projection.
 * @param paddingRatio A ratio of padding to add to the edges of the range computed from
 *     dsShadowProjection_addPoints().
 * @param minSize The minimum size along a dimension of the bounding box for the shadows.
 * @return False if there's no matrix to compute. This can be because either parameter is NULL or
 *     no points were added to the shadow projection.
 */
DS_RENDER_EXPORT bool dsShadowProjection_computeMatrix(dsMatrix44f* outMatrix,
	const dsShadowProjection* shadowProj, float paddingRatio, float minSize);

#ifdef __cplusplus
}
#endif
