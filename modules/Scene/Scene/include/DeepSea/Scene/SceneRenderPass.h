/*
 * Copyright 2019 Aaron Barany
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
#include <DeepSea/Scene/Types.h>
#include <DeepSea/Scene/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating scene render passes.
 * @see dsSceneRenderPass
 */

/**
 * @brief Gets the full size of the scene render pass.
 * @param framebuffer The name of the framebuffer.
 * @param clearValueCount The number of clear values.
 * @param subpassDrawLists The subpass draw lists.
 * @param subpassDrawListCount The number of subpass draw lists.
 * @return The full allocation size, or 0 if the subpass draw lists aren't valid.
 */
DS_SCENE_EXPORT size_t dsSceneRenderPass_fullAllocSize(const char* framebuffer,
	uint32_t clearValueCount, const dsSceneItemLists* subpassDrawLists,
	uint32_t subpassDrawListCount);

/**
 * @brief Creates a scene render pass.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the render pass with.
 * @param renderPass The core render pass this extends. This will take ownership of the render pass,
 *     and free it immediately if creation fails.
 * @param framebuffer The name of the framebuffer to use. This will be copied.
 * @param clearValues The clear values to use. This may be NULL if no attachment is set to clear.
 * @param clearValueCount The number of clear values. It is expected that this matches the number of
 *     attachments in renderPass or 0.
 * @param subpassDrawLists The draw lists for the subpasses. This will copy the arrays and take
 *     ownership of the draw lists themselves. The draw lists will be freed immediately if creation
 *     fails.
 * @param subpassDrawListCount The number of subpass draw lists. It is expected that this matches
 *     the number of subpasses in renderPass.
 * @return The scene render pass or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsSceneRenderPass* dsSceneRenderPass_create(dsAllocator* allocator,
	dsRenderPass* renderPass, const char* framebuffer, const dsSurfaceClearValue* clearValues,
	uint32_t clearValueCount, const dsSceneItemLists* subpassDrawLists,
	uint32_t subpassDrawListCount);

/**
 * @brief Destroys a scene render pass.
 */
DS_SCENE_EXPORT void dsSceneRenderPass_destroy(dsSceneRenderPass* renderPass);

#ifdef __cplusplus
}
#endif
