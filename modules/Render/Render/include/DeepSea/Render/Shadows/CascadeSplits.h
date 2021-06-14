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

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions to compute the splits for cascaded shadows.
 *
 * Cascaded shadows split the view frustum into multiple volumes, each of which will have its own
 * section in a shadow map. This is used to have hgiher precision for shadows nearer to the camera
 * and lower shadows closer to the camera. A factor that controls the exponential factor determines
 * how linear or exponential the split is: more exponential splits will have higher precision for
 * nearer shadows, but will also have more visible a boundary between splits.
 */

/**
 * @brief Computes the number of cascades to use for a scene.
 *
 * This can be used to control the number of cascades when the near and far planes are variable.
 * When the near and far planes are known (or at least known to be very distant) this function
 * can be skipped and a pre-determined number of cascades can be used instead.
 *
 * @remark errno will be set on failure.
 * @param near The near plane of the view frustum.
 * @param far The far plane of the view frustum.
 * @param maxFirstSplitDist The maximum distance for the first split.
 * @param expFactor Exponential factor in the range [0, 1], where 0 uses linear distances between
 *     the splits and 1 is fully exponential.
 * @param maxCascades The maximum number of cascades to allow.
 * @return The number of cascades to use or 0 if the parameters are invalid.
 */
DS_RENDER_EXPORT unsigned int dsComputeCascadeCount(float near, float far, float maxFirstSplitDist,
	float expFactor, unsigned int maxCascades);

/**
 * @brief Computes the distance to a cascade split.
 *
 * When computing the frustum for the cascade this will correspond to the far plane.
 *
 * @remark errno will be set on failure.
 * @param near The near plane of the view frustum.
 * @param far The far plane of the view frustum.
 * @param expFactor Exponential factor in the range [0, 1], where 0 uses linear distances between
 *     the splits and 1 is fully exponential.
 * @param index The index of the cascade.
 * @param cascadeCount The number of cascades.
 * @return The distance to the cascade, or 0 if the parameters are invalid.
 */
DS_RENDER_EXPORT float dsComputeCascadeDistance(float near, float far, float expFactor,
	unsigned int index, unsigned int cascadeCount);

#ifdef __cplusplus
}
#endif
