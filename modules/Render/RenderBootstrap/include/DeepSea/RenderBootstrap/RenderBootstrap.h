/*
 * Copyright 2018 Aaron Barany
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
#include <DeepSea/RenderBootstrap/Export.h>
#include <DeepSea/RenderBootstrap/RendererIDs.h>
#include <DeepSea/RenderBootstrap/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for bootstrapping a renderer.
 */

/**
 * @brief Gets the name for a renderer.
 * @param type The type of the renderer.
 * @return The renderer name, or NULL if an invalid enum or dsRendererType_Default.
 */
DS_RENDERBOOTSTRAP_EXPORT const char* dsRenderBootstrap_rendererName(dsRendererType type);

/**
 * @brief Gets the type of a renderer by name.
 * @param name The name of the renderer.
 * @return The renderer type, or dsRendererType_Default if no such name was found.
 */
DS_RENDERBOOTSTRAP_EXPORT dsRendererType dsRenderBootstrap_rendererTypeFromName(const char* name);

/**
 * @brief Creates a renderer.
 * @remark errno will be set on failure.
 * @param type The type of renderer to create.
 * @param allocator The allocator to create the renderer with.
 * @param options The options for creating the renderer.
 * @return The
 */
DS_RENDERBOOTSTRAP_EXPORT dsRenderer* dsRenderBootstrap_createRenderer(dsRendererType type,
	dsAllocator* allocator, const dsRendererOptions* options);

#ifdef __cplusplus
}
#endif
