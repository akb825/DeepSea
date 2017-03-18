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
 * @brief Functions for creating and using shader material descriptions.
 *
 * A dsMaterialDesc is provided when creating a shader to aid in assigning material values. It is
 * also used to create material instances.
 *
 * The same material description may be used with multiple shaders, so long as all of the uniforms
 * of the shader are provided. It is still valid if extra material parameters are provided.
 *
 * It is encouraged to re-use the same material description for multiple shaders when they use
 * similar parameters. This allows materials to be shared across those shaders and may make
 * rendering more efficient.
 *
 * @see dsMaterialDesc
 */

/**
 * @brief Creates a material description.
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager to create the material description from.
 * @param allocator The allocator to create the material description with. If NULL, it will use the
 *     same allocator as the resource manager.
 * @param elements The elements that comprise the material.
 * @param elementCount The number of material elements.
 * @return The created material description, or NULL if it couldn't be created.
 */
DS_RENDER_EXPORT dsMaterialDesc* dsMaterialDesc_create(dsResourceManager* resourceManager,
	dsAllocator* allocator, const dsMaterialElement* elements, uint32_t elementCount);

/**
 * @brief Finds an element within a material description.
 * @param materialDesc The material description.
 * @param name The name of the variable.
 * @return The index of the element, or DS_UNKNOWN if not found. This can be used to index into the
 *     elements member of dsMaterialDesc or access the data within dsMaterial.
 */
DS_RENDER_EXPORT uint32_t dsMaterialDesc_findElement(const dsMaterialDesc* materialDesc,
	const char* name);

/**
 * @brief Destroys a material description.
 * @remark errno will be set on failure.
 * @param materialDesc The material description to destroy.
 * @return False if the material description couldn't be destroyed.
 */
DS_RENDER_EXPORT bool dsMaterialDesc_destroy(dsMaterialDesc* materialDesc);

#ifdef __cplusplus
}
#endif
