/*
 * Copyright 2026 Aaron Barany
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
 * @brief Functions for creating and manipulating render surface hints.
 *
 * This proivdes a higher level interface to choose what formats to use for the system backed render
 * surfaces and color space expected by the windowing system.
 *
 * @see dsRenderSurfaceHint
 */

/**
 * @brief Initializes a render surface hint with the default values.
 * @remark errno will be set on failure.
 * @param[out] hint The render surface hint to initialize.
 * @return False if hint is NULL.
 */
DS_RENDER_EXPORT bool dsRenderSurfaceHint_default(dsRenderSurfaceHint* hint);

/**
 * @brief Initializes a render surface hint with explicit formats.
 * @remark errno will be set on failure.
 * @param[out] hint The render surface hint to initialize.
 * @param colorFormat The color format to use. This must be a valid format.
 * @param depthStencilFormat The depth stencil format to use. This may be dsGfxFormat_Unknown to
 *     have no depth-stencil format.
 * @param colorSpace The color space to use.
 * @param explicit Whether to use the provided formats as-is. When false, a similar format may be
 *     chosen based on what is optimal for the implementation.
 * @return False if hint is NULL or a format is invalid.
 */
DS_RENDER_EXPORT bool dsRenderSurfaceHint_fromFormats(dsRenderSurfaceHint* hint,
	dsGfxFormat colorFormat, dsGfxFormat depthStencilFormat, dsRenderColorSpace colorSpace,
	bool explicit);

/**
 * @brief Checks whether the render surface hint can produce a valid color format.
 * @param hint The render surface hint.
 * @return Whether the hint is valid.
 */
DS_RENDER_EXPORT bool dsRenderSurfaceHint_isValid(const dsRenderSurfaceHint* hint);

/**
 * @brief Gets the color format based on the provided hint.
 * @param hint The render surface hint.
 * @param bgr Use formats with BGR ordering instead of RGB.
 * @param aligned Whether the values should be aligned, such as needing an alpha channel for 8 bit
 *     per channel formats.
 * @return The color format.
 */
DS_RENDER_EXPORT dsGfxFormat dsRenderSurfaceHint_colorFormat(
	const dsRenderSurfaceHint* hint, bool bgr, bool aligned);

/**
 * @brief Gets the depth/stencil format based on the provided hint.
 * @param hint The render surface hint.
 * @return The depth/stencil format.
 */
DS_RENDER_EXPORT dsGfxFormat dsRenderSurfaceHint_depthStencilFormat(
	const dsRenderSurfaceHint* hint);

#ifdef __cplusplus
}
#endif
