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

#include <DeepSea/Core/Config.h>
#include <DeepSea/Render/Resources/Types.h>
#include <DeepSea/Render/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating materials.
 *
 * Materials hold the values to set on the shader during rendering. A material may be used on any
 * shader that uses the same dsMaterialDesc instance, even if the shader code is different. Material
 * values that are used by the shader (declared as uniforms) are used, while the rest are ignored.
 *
 * Materials directly store the values that are declared as individual uniforms in the shaders.
 * The rest are handled by dsShaderVariableGroup or manual uniform blocks or buffers.
 * dsShaderVariableGroup, uniform blocks or buffers, and textures may optionally be set by
 * dsSharedMaterialValues instead for values that control the rendering rather than properties
 * of the material.
 *
 * @see dsMaterial
 */

/**
 * @brief Creates a material.
 * @param resourceManager The resource manager to create the material from.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the material description with. If NULL, it will use the
 *     same allocator as the resource manager.
 * @param description The description for the material.
 * @return The created material, or NULL if it couldn't be created.
 */
DS_RENDER_EXPORT dsMaterial* dsMaterial_create(dsResourceManager* resourceManager,
	dsAllocator* allocator,const dsMaterialDesc* description);

/**
 * @brief Gets the resource manager the material was created with.
 * @remark errno will be set on failure.
 * @param material The material.
 * @return The resource manager.
 */
DS_RENDER_EXPORT dsResourceManager* dsMaterial_getResourceManager(const dsMaterial* material);

/**
 * @brief Gets the material description.
 * @remark errno will be set on failure.
 * @param material The material.
 * @return The material description.
 */
DS_RENDER_EXPORT const dsMaterialDesc* dsMaterial_getDescription(const dsMaterial* material);

/**
 * @brief Gets the device material.
 *
 * This only has any real meaning for the renderer implementation.
 *
 * @param material The material.
 * @return The device material. This may be NULL if the implementation doesn't create one.
 */
DS_RENDER_EXPORT dsDeviceMaterial* dsMaterial_getDeviceMaterial(const dsMaterial* material);

/**
 * @brief Gets the data for an element of a primitive, vector, or matrix type.
 * @remark errno will be set on failure.
 * @param[out] outData The buffer to receive the data.
 * @param material The material to get the data from.
 * @param element The index of the element to get.oo
 * @param type The type of the data.
 * @param firstIndex The first index to get when the element is an array.
 * @param count The number of array indices to get from the element. This must be 1 if not an array.
 * @return False if the element couldn't be gotten.
 */
DS_RENDER_EXPORT bool dsMaterial_getElementData(void* outData, const dsMaterial* material,
	uint32_t element, dsMaterialType type, uint32_t firstIndex, uint32_t count);

/**
 * @brief Gets the raw internal data pointer for an element.
 * @remark errno will be set on failure.
 * @param material The material to get the data from.
 * @param element The element index.
 * @return The pointer to the element data, or NULL if invalid.
 */
DS_RENDER_EXPORT const void* dsMaterial_getRawElementData(const dsMaterial* material,
	uint32_t element);

/**
 * @brief Sets the data for an element of a primitive, vector, or matrix type.
 * @remark errno will be set on failure.
 * @param material The material to set the data on.
 * @param element The index of the element to set.
 * @param data The data to set.
 * @param type The type of the data.
 * @param firstIndex The first index to set when the element is an array.
 * @param count The number of array indices to set in the element. This must be 1 if not an array.
 * @return False if the element couldn't be set.
 */
DS_RENDER_EXPORT bool dsMaterial_setElementData(dsMaterial* material, uint32_t element,
	const void* data, dsMaterialType type, uint32_t firstIndex, uint32_t count);

/**
 * @brief Gets the texture data for a material element.
 * @param material The material to get the texture from.
 * @param element The index of the element to get.
 * @return The texture, or NULL if not found or unset.
 */
DS_RENDER_EXPORT dsTexture* dsMaterial_getTexture(const dsMaterial* material, uint32_t element);

/**
 * @brief Sets the texture data for a material element.
 * @remark errno will be set on failure.
 * @param material The material to set the texture on.
 * @param element The index of the element to set.
 * @param texture The texture to set.
 * @return Flase if the parameters are invalid or the element isn't a texture.
 */
DS_RENDER_EXPORT bool dsMaterial_setTexture(dsMaterial* material, uint32_t element,
	dsTexture* texture);

/**
 * @brief Gets the texture buffer data for a material element.
 * @param[out] outFormat The texture format to interpret the buffer data. This may be NULL.
 * @param[out] outOffset The offset into the buffer. This may be NULL.
 * @param[out] outCount The number of texels for the buffer. This may be NULL.
 * @param material The material to get the buffer from.
 * @param element The index of the element to get.
 * @return The buffer, or NULL if not found or unset.
 */
DS_RENDER_EXPORT dsGfxBuffer* dsMaterial_getTextureBuffer(dsGfxFormat* outFormat, size_t* outOffset,
	size_t* outCount, const dsMaterial* material, uint32_t element);

/**
 * @brief Sets the texture buffer data for a material element.
 * @remark errno will be set on failure.
 * @param material The material to set the buffer on.
 * @param element The index of the element to set.
 * @param buffer The buffer to set.
 * @param format The texture format to interpret the buffer data.
 * @param offset The offset into the buffer.
 * @param count The number of texels for the buffer.
 * @return Flase if the parameters are invalid or the element isn't a buffer.
 */
DS_RENDER_EXPORT bool dsMaterial_setTextureBuffer(dsMaterial* material, uint32_t element,
	dsGfxBuffer* buffer, dsGfxFormat format, size_t offset, size_t count);

/**
 * @brief Gets the shader variable group data for a material element.
 * @param material The material to get the shader variable group from.
 * @param element The index of the element to get.
 * @return The shader variable group, or NULL if not found or unset.
 */
DS_RENDER_EXPORT dsShaderVariableGroup* dsMaterial_getVariableGroup(const dsMaterial* material,
	uint32_t element);

/**
 * @brief Sets the shader variable group data for a material element.
 * @remark errno will be set on failure.
 * @param material The material to set the shader variable group on.
 * @param element The index of the element to set.
 * @param group The shader variable group to set.
 * @return Flase if the parameters are invalid or the element isn't a shader variable group.
 */
DS_RENDER_EXPORT bool dsMaterial_setVariableGroup(dsMaterial* material, uint32_t element,
	dsShaderVariableGroup* group);

/**
 * @brief Gets the buffer data for a material element.
 * @param[out] outOffset The offset into the buffer. This may be NULL.
 * @param[out] outSize The size to use within the buffer. This may be NULL.
 * @param material The material to get the buffer from.
 * @param element The index of the element to get.
 * @return The buffer, or NULL if not found or unset.
 */
DS_RENDER_EXPORT dsGfxBuffer* dsMaterial_getBuffer(size_t* outOffset, size_t* outSize,
	const dsMaterial* material, uint32_t element);

/**
 * @brief Sets the buffer data for a material element.
 * @remark errno will be set on failure.
 * @param material The material to set the buffer on.
 * @param element The index of the element to set.
 * @param buffer The buffer to set.
 * @param offset The offset into the buffer.
 * @param size The size to use within the buffer.
 * @return Flase if the parameters are invalid or the element isn't a buffer.
 */
DS_RENDER_EXPORT bool dsMaterial_setBuffer(dsMaterial* material, uint32_t element,
	dsGfxBuffer* buffer, size_t offset, size_t size);

/**
 * @brief Commits changes to the material values.
 *
 * This will ensure any changes are reflected in shaders when drawing geometry. If this isn't
 * called, changes may not be applied.
 *
 * @remark This shouldn't be used across multiple threads. If different values are required across
 * threads, consider dsSharedMaterialValues.
 *
 * @remark errno will be set on failure.
 * @param material The material to commit the changes for.
 * @param commandBuffer The command buffer to commit the changes on.
 * @return False if the values couldn't be committed.
 */
DS_RENDER_EXPORT bool dsMaterial_commit(dsMaterial* material, dsCommandBuffer* commandBuffer);

/**
 * @brief Destroys a material.
 * @param material The material to destroy.
 */
DS_RENDER_EXPORT void dsMaterial_destroy(dsMaterial* material);

#ifdef __cplusplus
}
#endif
