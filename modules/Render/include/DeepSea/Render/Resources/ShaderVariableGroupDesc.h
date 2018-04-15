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
 * @brief Functions for creating and using shader variable group descriptions.
 * @see dsShaderVariableGroupDesc
 */

/**
 * @brief Creates a shader variable group description.
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager to create the shader variable group description from.
 * @param allocator The allocator to create the shader variable group description with. If NULL, it
 *     will use the same allocator as the resource manager.
 * @param elements The elements that comprise the shader variable group. This array will be copied.
 * @param elementCount The number of material elements. This must not be empty.
 * @return The created shader variable group description, or NULL if it couldn't be created.
 */
DS_RENDER_EXPORT dsShaderVariableGroupDesc* dsShaderVariableGroupDesc_create(
	dsResourceManager* resourceManager, dsAllocator* allocator,
	const dsShaderVariableElement* elements, uint32_t elementCount);

/**
 * @brief Finds an element within a shader variable description.
 * @param groupDesc The shader variable group description.
 * @param name The name of the variable.
 * @return The index of the element, or DS_MATERIAL_UNKNOWN if not found. This can be used to index
 *     into the elements member of dsShaderVariableGroupDesc or access the data within
 *     dsShaderVariableGroup.
 */
DS_RENDER_EXPORT uint32_t dsShaderVariableGroupDesc_findElement(
	const dsShaderVariableGroupDesc* groupDesc, const char* name);

/**
 * @brief Destroys a shader variable group description.
 * @remark errno will be set on failure.
 * @param groupDesc The shader variable group description to destroy.
 * @return False if the shader variable group description couldn't be destroyed.
 */
DS_RENDER_EXPORT bool dsShaderVariableGroupDesc_destroy(dsShaderVariableGroupDesc* groupDesc);

#ifdef __cplusplus
}
#endif
