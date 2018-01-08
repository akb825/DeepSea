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
 * @brief Functions for creating and manipulating gradients.
 * @see dsGradient
 */

/**
 * @brief Gets the full allocation size of a gradient.
 * @param stopCount The number of gradient stops.
 * @return The full allocation size.
 */
DS_VECTORDRAW_EXPORT size_t dsGradient_fullAllocSize(uint32_t stopCount);

/**
 * @brief Creates a gradient.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the gradient with.
 * @param stops The gradient stops.
 * @param stopCount The number of stops.
 * @return The gradient, or NULL if it couldn't be created.
 */
DS_VECTORDRAW_EXPORT dsGradient* dsGradient_create(dsAllocator* allocator,
	const dsGradientStop* stops, uint32_t stopCount);

/**
 * @brief Checks that a gradient is valid.
 *
 * This can be used to make sure a gradient is valid after modifying the stops. The positions must
 * be monotonically increasing and in the range [0, 1].
 *
 * @param gradient The gradient to check.
 * @return True if the gradient is valid.
 */
DS_VECTORDRAW_EXPORT bool dsGradient_isValid(const dsGradient* gradient);

/**
 * @brief Evaluates a gradient.
 * @param gradient The gradient to evaluate.
 * @param t The position in the gradient in the range [0, 1].
 */
DS_VECTORDRAW_EXPORT dsColor dsGradient_evaluate(const dsGradient* gradient, float t);

/**
 * @brief Destroys a gradient.
 * @param gradient The gradient to destroy.
 */
DS_VECTORDRAW_EXPORT void dsGradient_destroy(dsGradient* gradient);

#ifdef __cplusplus
}
#endif
