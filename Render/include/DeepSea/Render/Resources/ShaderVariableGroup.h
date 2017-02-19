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
#include <DeepSea/Render/Resources/Types.h>
#include <DeepSea/Render/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating shader variable groups.
 *
 * This allows conditional usage of buffers for uniform blocks. When uniform blocks are supported,
 * a graphics buffer will be used internally. Additionally, setting of values may be done on the
 * command buffer, which allows for the values to change in-between draw calls when creating command
 * buffers across multiple threads.
 *
 * @see dsShaderVariableGroup
 */

/**
 * @brief Gets the size of dsShaderVariableGroup.
 * @return The size of dsShaderVariableGroup.
 */
DS_RENDER_EXPORT size_t dsShaderVariableGroup_sizeof(void);

/**
 * @brief Gets the full allocated size of dsShaderVariableGroup.
 *
 * This doesn't include the allocated size of the dsGfxBuffer used on targets that support uniform
 * blocks. A separate allocator may be used for the dsGfxBuffer instance so the rest of the memory
 * may be pre-allocated if desired.
 *
 * @param resourceManager The resource manager the shader variable group will be allocated with.
 * @param description The description for the shader variable group.
 * @param commitType How the variables will be committed to the GPU.
 * @return The full allocated size of dsShaderVariableGroup.
 */
DS_RENDER_EXPORT size_t dsShaderVariableGroup_fullAllocSize(
	const dsResourceManager* resourceManager, const dsShaderVariableGroupDesc* description,
	dsShaderCommitType commitType);

/**
 * @brief Returns whether or not a graphics buffer will be used.
 * @param resourceManager The resource manager.
 * @return True if a graphics buffer will be used.
 */
DS_RENDER_EXPORT bool dsShaderVariableGroup_useGfxBuffer(const dsResourceManager* resourceManager);

/**
 * @brief Creates a shader variable group.
 * @param resourceManager The resource manager to create the shader variable group from.
 * @param allocator The allocator to create the shader variable group with. If NULL, it will use the
 *     same allocator as the resource manager.
 * @param gfxBufferAllocator The allocator to create the graphics buffer with. (on targets that
 *     support uniform blocks) If NULL, it will use the same allocator as the resource manager.
 * @param description The description for the shader variable group.
 * @param commitType How the variables will be committed to the GPU.
 * @return The created shader variable group, or NULL if it couldn't be created. errno will be set
 *     to an appropriate value on failure.
 */
DS_RENDER_EXPORT dsShaderVariableGroup* dsShaderVariableGroup_create(
	dsResourceManager* resourceManager, dsAllocator* allocator, dsAllocator* gfxBufferAllocator,
	const dsShaderVariableGroupDesc* description, dsShaderCommitType commitType);

/**
 * @brief Gets the shader variable group description.
 * @param group The shader variable group.
 * @return The shader vairable group description.
 */
DS_RENDER_EXPORT const dsShaderVariableGroupDesc* dsShaderVariableGroup_getDescription(
	const dsShaderVariableGroup* group);

/**
 * @brief Sets the data for an element.
 *
 * This may allocate memory if all of the following conditions are true:
 * - Uniform blocks are supported.
 * - The commit type is set to immediate.
 * - The packing of the data doesn't match the packing on the GPU. (e.g. array of vec3, non-square
 *   matrices)
 * - The size of the data is > 1 KB.
 * - The amount of data set is larger than previous calls for the same shader variable group that
 *   meets the above criteria.
 *
 * The memory will be allocated with the allocator that the shader variable group was created with,
 * or the resource manager's allocator if it was created with an allocator that doesn't free memory.
 * (e.g. dsBufferAllocator)
 *
 * In order to avoid any memory allocations under any situation, you can set the commit type to
 * batched. More memory in total will be allocated, but it will guarantee it's all up front.
 *
 * @param commandBuffer The command buffer. Whether or not this is used depends on the
 *     implementation and should not be relied on to be executed with the command buffer.
 * @param group The shader variable group to set the data on.
 * @param element The index of the element to set.
 * @param data The data to set.
 * @param type The type of the data.
 * @param firstIndex The first index to set when the element is an array.
 * @param count The number of array indices to set in the element. This must be 1 if not an array.
 * @return False if the element couldn't be set.
 */
DS_RENDER_EXPORT bool dsShaderVariableGroup_setElementData(dsCommandBuffer* commandBuffer,
	dsShaderVariableGroup* group, uint32_t element, const void* data, dsMaterialType type,
	uint32_t firstIndex, uint32_t count);

/**
 * @brief Gets the graphics buffer for the shader variable data.
 *
 * This is generally used by the renderer implementation to bind to the shader.
 *
 * @param group The shader variable group.
 * @return The graphics buffer, or NULL if not using graphics buffers.
 */
DS_RENDER_EXPORT dsGfxBuffer* dsShaderVariableGroup_getGfxBuffer(
	const dsShaderVariableGroup* group);

/**
 * @brief Gets the data for an element when a graphics buffer isn't used.
 *
 * This is generally used by the renderer implementation to bind to the shader.
 *
 * @param group The shader variable group.
 * @param element The element index.
 * @return The pointer to the element data, or NULL if using graphics buffers.
 */
DS_RENDER_EXPORT const void* dsShaderVariableGroup_getRawElementData(
	const dsShaderVariableGroup* group, uint32_t element);

/**
 * @brief Gets whether or not an element is dirty.
 *
 * This should be used by the renderer implementation when uniform blocks aren't supported to avoid
 * copying data to uniforms unnecessarily. The dirty flags will be cleared with
 * dsShaderVariableGroup_commit().
 *
 * @param group The shader variable group.
 * @param element The element index.
 * @return True if the element is dirty.
 */
DS_RENDER_EXPORT bool dsShaderVariableGroup_isElementDirty(const dsShaderVariableGroup* group,
	uint32_t element);

/**
 * @brief Commits any pending changes to the shader variable group to the GPU.
 *
 * This is used by the renderer implementation. If created with dsShaderCommitType_Batched, this
 * will perform the actual copy to the GPU buffer.
 *
 * If uniform blocks aren't supported, this clears the dirty flags. The renderer implementation
 * should do any checks for copying individual unfirom data before calling this function.
 *
 * @param commandBuffer The command buffer. Whether or not this is used depends on the
 *     implementation and should not be relied on to be executed with the command buffer.
 * @param group The shader variable group to commit changes for.
 * @return False if the variables couldn't be committed.
 */
DS_RENDER_EXPORT bool dsShaderVariableGroup_commit(dsCommandBuffer* commandBuffer,
	dsShaderVariableGroup* group);

/**
 * @brief Destroys a shader variable group.
 * @param group The shader variable group to destroy.
 * @return False if the shader variable group couldn't be destroyed.
 */
DS_RENDER_EXPORT bool dsShaderVariableGorup_destroy(dsShaderVariableGroup* group);

#ifdef __cplusplus
}
#endif
