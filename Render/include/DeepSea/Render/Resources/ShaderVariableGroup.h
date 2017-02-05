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
 * This allows conditional usage of buffers for shader blocks. When shader blocks are supported,
 * a graphics buffer will be used internally. Additionally, setting of values may be done on the
 * command buffer, which allows for the values to change in-between draw calls when creating command
 * buffers across multiple threads.
 *
 * @see dsShaderVariableGroup
 */

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
 * @param description The description for the shader variable group.
 * @return The created shader variable group, or NULL if it couldn't be created. errno will be set
 *     to an appropriate value on failure.
 */
DS_RENDER_EXPORT dsShaderVariableGroup* dsShaderVariableGroup_create(
	dsResourceManager* resourceManager, dsAllocator* allocator,
	const dsShaderVariableGroupDesc* description);

/**
 * @brief Gets the shader variable group description.
 * @param group The shader variable group.
 * @return The shader vairable group description.
 */
DS_RENDER_EXPORT const dsShaderVariableGroupDesc* dsShaderVariableGroup_getDescription(
	const dsShaderVariableGroup* group);

/**
 * @brief Sets the data for an element.
 * @param commandBuffer The command buffer. Whether or not this is used depends on the
 *     implementation and should not be relied on to be executed with the command buffer.
 * @param group The shader variable group to set the index on.
 * @param element The index of the element to set.
 * @param data The data to set.
 * @param type The type of the data.
 * @param firstIndex The first index to set when the element is an array.
 * @param count The number of array indices to set to set in the element.
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
DS_RENDER_EXPORT const void* dsShaderVariableGroup_getElementData(
	const dsShaderVariableGroup* group, uint32_t element);

/**
 * @brief Destroys a shader variable group.
 * @param group The shader variable group to destroy.
 * @return False if the shader variable group couldn't be destroyed.
 */
DS_RENDER_EXPORT bool dsShaderVariableGorup_destroy(dsShaderVariableGroup* group);

#ifdef __cplusplus
}
#endif
