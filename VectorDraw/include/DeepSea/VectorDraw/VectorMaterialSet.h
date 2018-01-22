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
 * @brief Functions for creating and manipulating material set.
 *
 * The materials here will be referenced by name when creating vector images. The material
 * parameters may be changed on the fly to affect the images that use them.
 *
 * @see dsVectorMaterialSet
 */

/**
 * @brief Constant for a vector material not being found.
 */
#define DS_VECTOR_MATERIAL_NOT_FOUND (uint32_t)-1

/**
 * @brief Gets the full allocation size of a vector material set.
 *
 * This doesn't include the size for the texture.
 *
 * @param maxMaterials the maximum number of materials that can be held.
 * @return The full allocation size.
 */
DS_VECTORDRAW_EXPORT size_t dsVectorMaterialSet_fullAllocSize(uint32_t maxMaterials);

/**
 * @brief Creates a material set.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the material set with.
 * @param resourceManager The resource manager to create the texture with. Floating point textures
 *     must be supported.
 * @param textureAllocator The allocator to create the texture with. If NULL, it will use the
 *     allocator for the material set.
 * @param maxMaterials the maximum number of materials that can be held.
 * @return The material set, or NULL if it couldn't be created.
 */
DS_VECTORDRAW_EXPORT dsVectorMaterialSet* dsVectorMaterialSet_create(dsAllocator* allocator,
	dsResourceManager* resourceManager, dsAllocator* textureAllocator, uint32_t maxMaterials);

/**
 * @brief Gets the number of remaining materials that can be set.
 * @param materials The material set.
 * @return The number of remaining materials.
 */
DS_VECTORDRAW_EXPORT uint32_t dsVectorMaterialSet_getRemainingMaterials(
	const dsVectorMaterialSet* materials);

/**
 * @brief Adds a material to a material set.
 * @remark errno will be set on failure.
 * @param materials The material set.
 * @param name The name of the material. The length, including the null terminator, must not exceed
 *     DS_MAX_VECTOR_RESOURCE_NAME_LENGTH.
 * @param material The material to add.
 * @param ownGradient True to take ownership of the material gradient.
 * @return False if the material couldn't be added.
 */
DS_VECTORDRAW_EXPORT bool dsVectorMaterialSet_addMaterial(dsVectorMaterialSet* materials,
	const char* name, const dsVectorMaterial* material, bool ownGradient);

/**
 * @brief Sets the color for a material.
 *
 * This is only valid if the material was previously set as a color material.
 *
 * @remark errno will be set on failure.
 * @param materials The material set.
 * @param name The name of the material.
 * @param color The new color.
 * @return False if the color couldn't be set.
 */
DS_VECTORDRAW_EXPORT bool dsVectorMaterialSet_setMaterialColor(dsVectorMaterialSet* materials,
	const char* name, dsColor color);

/**
 * @brief Sets the gradient for a material.
 *
 * This is only valid if the material was previously set as a linear or radial gradient.
 *
 * @remark errno will be set on failure.
 * @param materials The material set.
 * @param name The name of the material.
 * @param gradient The new gradient. If it's the previously set gradient, it will trigger the
 *     gradient to be re-calculated. (such as if a stop was adjusted)
 * @param own True to take ownership of the material gradient.
 * @return False if the color couldn't be set.
 */
DS_VECTORDRAW_EXPORT bool dsVectorMaterialSet_setMaterialGradient(dsVectorMaterialSet* materials,
	const char* name, const dsGradient* gradient, bool own);

/**
 * @brief Replaces a previously set material.
 * @remark errno will be set on failure.
 * @param materials The material set.
 * @param name The name of the material.
 * @param material The new material to set.
 * @param ownGradient True to take ownership of the material gradient.
 * @return False if the color couldn't be set.
 */
DS_VECTORDRAW_EXPORT bool dsVectorMaterialSet_setMaterial(dsVectorMaterialSet* materials,
	const char* name, const dsVectorMaterial* material, bool ownGradient);

/**
 * @brief Finds a material in the material set.
 * @param materials The material set.
 * @param name The name of the material.
 * @return The material or NULL if it couldn't be found.
 */
DS_VECTORDRAW_EXPORT const dsVectorMaterial* dsVectorMaterialSet_findMaterial(
	const dsVectorMaterialSet* materials, const char* name);

/**
 * @brief Finds the index of a material.
 * @param materials The material set.
 * @param name The name of the material to find.
 * @return The index of the material or DS_VECTOR_MATERIAL_NOT_FOUND if not found.
 */
DS_VECTORDRAW_EXPORT uint32_t dsVectorMaterialSet_findMaterialIndex(
	const dsVectorMaterialSet* materials, const char* name);

/**
 * @brief Updates the material set, committing any changes to the textures.
 * @remark This may not be called within a render pass.
 * @remark errno will be set on failure.
 * @param materials The material set.
 * @param commandBuffer The command buffer to place the commands on.
 * @return False if the material set couldn't be updated.
 */
DS_VECTORDRAW_EXPORT bool dsVectorMaterialSet_update(dsVectorMaterialSet* materials,
	dsCommandBuffer* commandBuffer);

/**
 * @brief Gets the texture that contains the material colors.
 * @param materials The material set.
 * @return The color texture.
 */
DS_VECTORDRAW_EXPORT dsTexture* dsVectorMaterialSet_getColorTexture(
	const dsVectorMaterialSet* materials);

/**
 * @brief Gets the texture that contains the material information.
 * @param materials The material set.
 * @return The info texture.
 */
DS_VECTORDRAW_EXPORT dsTexture* dsVectorMaterialSet_getInfoTexture(
	const dsVectorMaterialSet* materials);

/**
 * @brief Destroys a material set.
 * @remark errno will be set on failure.
 * @param materials The material set.
 * @return False if the resources couldn't be destroyed.
 */
DS_VECTORDRAW_EXPORT bool dsVectorMaterialSet_destroy(dsVectorMaterialSet* materials);

#ifdef __cplusplus
}
#endif
