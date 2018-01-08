/*
 * Copyright 2017 Aaron Barany
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
#include <DeepSea/VectorDraw/Export.h>
#include <DeepSea/VectorDraw/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for manipulating vector materials.
 * @remark This won't free any gradients stored within the material. It is the responsibility of the
 *     caller to manage the resources for gradients.
 * @see dsVectorMaterial
 */

/**
 * @brief Sets a vector material as a color.
 * @remark errno will be set on failure.
 * @param material The material to set the color on.
 * @param color The color to set.
 * @return False if the color couldn't be set.
 */
DS_VECTORDRAW_EXPORT bool dsVectorMaterial_setColor(dsVectorMaterial* material, dsColor color);

/**
 * @brief Sets a vector material as a linear gradient.
 * @remark errno will be set on failure.
 * @param material The material to set the gradient on.
 * @param gradient The gradient.
 * @param start The start position of the gradient.
 * @param end The end position of the gradient.
 * @param edge What to do at the edge of the gradient.
 * @param coordinateSpace The coordinate space of the gradient.
 * @param transform The transform, or NULL for identity.
 * @return False if the gradient is invalid.
 */
DS_VECTORDRAW_EXPORT bool dsVectorMaterial_setLinearGradient(dsVectorMaterial* material,
	const dsGradient* gradient, const dsVector2f* start, const dsVector2f* end, dsGradientEdge edge,
	dsVectorElementSpace coordinateSpace, const dsMatrix33f* transform);

/**
 * @brief Sets a vector material as a radial gradient.
 * @remark errno will be set on failure.
 * @param material The material to set the gradient on.
 * @param gradient The gradient.
 * @param center The center of the gradient.
 * @param radius The radius of the circle for the gradient.
 * @param focus The focus point of the gradient. This can be set to offset the gradeint from the
 *     center.
 * @param focusRadius The radius of the focus in the range [0, 1] as a portion of the overall
 *     radius.
 * @param edge What to do at the edge of the gradient.
 * @param coordinateSpace The coordinate space of the gradient.
 * @param transform The transform, or NULL for identity.
 * @return False if the gradient is invalid.
 */
DS_VECTORDRAW_EXPORT bool dsVectorMaterial_setRadialGradient(dsVectorMaterial* material,
	const dsGradient* gradient, const dsVector2f* center, float radius, const dsVector2f* focus,
	float focusRadius, dsGradientEdge edge, dsVectorElementSpace coordinateSpace,
	const dsMatrix33f* transform);

/**
 * @brief Gets the gradient attached to a material.
 * @param material The material.
 * @return The gradient or NULL if the material doesn't contain a gradient.
 */
DS_VECTORDRAW_EXPORT const dsGradient* dsVectorMaterial_getGradient(
	const dsVectorMaterial* material);

/**
 * @brief Sets the gradient attached to a material.
 *
 * This is only valid if already set as a linear or radial gradient, replacing the previously set
 * gradient.
 *
 * @remark errno will be set on failure.
 * @param material The material.
 * @param gradient The gradient to set.
 * @return False if the gradient couldn't be set.
 */
DS_VECTORDRAW_EXPORT bool dsVectorMaterial_setGradient(dsVectorMaterial* material,
	const dsGradient* gradient);

#ifdef __cplusplus
}
#endif
