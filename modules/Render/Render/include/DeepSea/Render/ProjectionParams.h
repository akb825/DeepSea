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
 * @brief Functions to set projection parameters and create a projection matrix.
 * @see dsProjectionParams
 */

/**
 * @brief Makes an orthographic projection.
 * @remark errno will be set on failure.
 * @param params The projection parameters to set.
 * @param left The left plane.
 * @param right The right plane.
 * @param bottom The bottom plane.
 * @param top The top plane.
 * @param near The near plane.
 * @param far The far plane.
 * @return False if the parameters are invalid.
 */
DS_RENDER_EXPORT bool dsProjectionParams_makeOrtho(dsProjectionParams* params, float left,
	float right, float bottom, float top, float near, float far);

/**
 * @brief Makes a frustum projection.
 * @remark errno will be set on failure.
 * @param params The projection parameters to set.
 * @param left The left plane.
 * @param right The right plane.
 * @param bottom The bottom plane.
 * @param top The top plane.
 * @param near The near plane.
 * @param far The far plane. This may be INFINITY.
 * @return False if the parameters are invalid.
 */
DS_RENDER_EXPORT bool dsProjectionParams_makeFrustum(dsProjectionParams* params, float left,
	float right, float bottom, float top, float near, float far);

/**
 * @brief Makes a perspective projection.
 * @remark errno will be set on failure.
 * @param params The projection parameters to set.
 * @param fovy The field of view in the Y direction in radians.
 * @param aspect The aspect ratio as X/Y.
 * @param near The near plane.
 * @param far The far plane. This may be INFINITY.
 * @return False if the parameters are invalid.
 */
DS_RENDER_EXPORT bool dsProjectionParams_makePerspective(dsProjectionParams* params, float fovy,
	float aspect, float near, float far);

/**
 * @brief Creates a matrix from the projection parameters.
 * @remark errno will be set on failure.
 * @param[out] result The projection matrix.
 * @param params The projection parameters.
 * @param renderer The renderer the matrix will be used with.
 * @return False if the parameters are invalid.
 */
DS_RENDER_EXPORT bool dsProjectionParams_createMatrix(dsMatrix44f* result,
	const dsProjectionParams* params, const dsRenderer* renderer);

#ifdef __cplusplus
}
#endif
