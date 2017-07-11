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
 * a graphics buffer will be used internally. It will minimize the number of calls to copy the data
 * to GPU memory at the expense of using a separate buffer on the CPU to hold the current values.
 * This will be more efficient when updating multiple small values, but would be less efficient when
 * working with large buffers that would be updated in one shot. In this case, consider manually
 * managing the uniform block with a dsGfxBuffer.
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
 * @return The full allocated size of dsShaderVariableGroup.
 */
DS_RENDER_EXPORT size_t dsShaderVariableGroup_fullAllocSize(
	const dsResourceManager* resourceManager, const dsShaderVariableGroupDesc* description);

/**
 * @brief Returns whether or not a graphics buffer will be used.
 * @param resourceManager The resource manager.
 * @return True if a graphics buffer will be used.
 */
DS_RENDER_EXPORT bool dsShaderVariableGroup_useGfxBuffer(const dsResourceManager* resourceManager);

/**
 * @brief Creates a shader variable group.
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager to create the shader variable group from.
 * @param allocator The allocator to create the shader variable group with. If NULL, it will use the
 *     same allocator as the resource manager.
 * @param gfxBufferAllocator The allocator to create the graphics buffer with. (on targets that
 *     support uniform blocks) If NULL, it will use the same allocator as the resource manager.
 * @param description The description for the shader variable group.
 * @return The created shader variable group, or NULL if it couldn't be created.
 */
DS_RENDER_EXPORT dsShaderVariableGroup* dsShaderVariableGroup_create(
	dsResourceManager* resourceManager, dsAllocator* allocator, dsAllocator* gfxBufferAllocator,
	const dsShaderVariableGroupDesc* description);

/**
 * @brief Gets the shader variable group description.
 * @remark errno will be set on failure.
 * @param group The shader variable group.
 * @return The shader vairable group description.
 */
DS_RENDER_EXPORT const dsShaderVariableGroupDesc* dsShaderVariableGroup_getDescription(
	const dsShaderVariableGroup* group);

/**
 * @brief Sets the data for an element.
 * @remark errno will be set on failure.
 * @param group The shader variable group to set the data on.
 * @param element The index of the element to set.
 * @param data The data to set.
 * @param type The type of the data.
 * @param firstIndex The first index to set when the element is an array.
 * @param count The number of array indices to set in the element. This must be 1 if not an array.
 * @return False if the element couldn't be set.
 */
DS_RENDER_EXPORT bool dsShaderVariableGroup_setElementData(dsShaderVariableGroup* group,
	uint32_t element, const void* data, dsMaterialType type, uint32_t firstIndex, uint32_t count);

/**
 * @brief Commits any pending changes to the shader variable group to the GPU.
 *
 * If uniform blocks are supported, this will perform the actual copy to the GPU buffer. If uniform
 * blocks aren't supported, this increments the commit count. Render implementations can keep track
 * of the commit count to avoid uploading all uniforms when updating a dsShaderVariableGRoup used
 * as part of dsVolatileMaterialValues.
 *
 * @remark errno will be set on failure.
 * @param commandBuffer The command buffer. Whether or not this is used depends on the
 *     implementation and should not be relied on to be executed with the command buffer.
 * @param group The shader variable group to commit changes for.
 * @return False if the variables couldn't be committed.
 */
DS_RENDER_EXPORT bool dsShaderVariableGroup_commit(dsCommandBuffer* commandBuffer,
	dsShaderVariableGroup* group);

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
 * copying data to uniforms unnecessarily.
 *
 * @param group The shader variable group.
 * @param element The element index.
 * @param commitCount The commit count for the last set value.
 * @return True if the element is dirty.
 */
DS_RENDER_EXPORT bool dsShaderVariableGroup_isElementDirty(const dsShaderVariableGroup* group,
	uint32_t element, uint64_t commitCount);

/**
 * @brief Gets the commit count when uniform blocks aren't supported.
 *
 * The implementation can keep track of the commit count to avoid uploading setting uniforms that
 * haven't changed value.
 */
DS_RENDER_EXPORT uint64_t dsShaderVariableGroup_getCommitCount(const dsShaderVariableGroup* group);

/**
 * @brief Destroys a shader variable group.
 * @remark errno will be set on failure.
 * @param group The shader variable group to destroy.
 * @return False if the shader variable group couldn't be destroyed.
 */
DS_RENDER_EXPORT bool dsShaderVariableGroup_destroy(dsShaderVariableGroup* group);

#ifdef __cplusplus
}
#endif
