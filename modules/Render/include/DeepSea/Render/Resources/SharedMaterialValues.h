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
 * @brief Functions for creating and using shared material values.
 *
 * This holds a set of values to be used for shared material elements indexed by name. Shared
 * material values are texture, image, and buffer material values set as global or instance binding
 * within dsMaterialDesc. This allows values for the current rendering state to be stored separately
 * from the material properties. Separate instances can be used for different render passes, draw
 * threads, etc. to remain independent between multiple uses of the material, and the values stored
 * in this may be changed in-between draw calls.
 *
 * The cost of updating a value is dependent on the underlying graphics API and driver. Global
 * bindings are intended to change rarely, while instance bindings are intended to change more
 * often. In the case of Vulkan, which is the most explicit in how shader variables are managed, the
 * cheapest change is to update the offset for a uniform block or uniform buffer while keeping the
 * underlying buffer the same. When possible, this is the best update to change in-between draw
 * calls for instance bindings.
 *
 * Lookups into this will be frequent, so as a result the index is done by pre-hashing the name. You
 * may either access the elements by name or by the ID, which is the hash of the name. (by calling
 * dsHashString())
 *
 * This will prevent setting a value when a value is set with the same name but a different type.
 * In this case, errno will be set to EPERM. errno will be set to ENOMEM if the maximum number of
 * values is exceeded.
 *
 * @see dsSharedMaterialValues
 */

/**
 * @brief The default maximum number of shared material values.
 */
#define DS_DEFAULT_MAX_SHARED_MATERIAL_VALUES 100U

/**
 * @brief Gets the size of dsSharedMaterialValues.
 * @return The size of dsSharedMaterialValues.
 */
DS_RENDER_EXPORT size_t dsSharedMaterialValues_sizeof(void);

/**
 * @brief Gets the full allocated size of dsSharedMaterialValues.
 * @param maxValues The maximum number of values that can be stored.
 * @return The full allocated size of dsSharedMaterialValues.
 */
DS_RENDER_EXPORT size_t dsSharedMaterialValues_fullAllocSize(unsigned int maxValues);

/**
 * @brief Creates a shared material values instance.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the shared material values with.
 * @param maxValues The maximum number of values to use.
 */
DS_RENDER_EXPORT dsSharedMaterialValues* dsSharedMaterialValues_create(dsAllocator* allocator,
	unsigned int maxValues);

/**
 * @brief Gets the number of values stored in a shared material values instance.
 * @param values The shared material values.
 * @return The number of values.
 */
DS_RENDER_EXPORT unsigned int dsSharedMaterialValues_getValueCount(
	const dsSharedMaterialValues* values);

/**
 * @brief Gets the maximum number of values that can be stored in a shared material values
 *     instance.
 * @param values The shared material values.
 * @return The maximum number of values.
 */
DS_RENDER_EXPORT unsigned int dsSharedMaterialValues_getMaxValueCount(
	const dsSharedMaterialValues* values);

/**
 * @brief Gets a texture value by name.
 * @param values The shared material values.
 * @param name The name of the texture.
 * @return The texture, or NULL if not found or unset.
 */
DS_RENDER_EXPORT dsTexture* dsSharedMaterialValues_getTextureName(
	const dsSharedMaterialValues* values, const char* name);

/**
 * @brief Gets a texture value by ID.
 * @param values The shared material values.
 * @param nameID The hash of the texture name.
 * @return The texture, or NULL if not found or unset.
 */
DS_RENDER_EXPORT dsTexture* dsSharedMaterialValues_getTextureID(
	const dsSharedMaterialValues* values, uint32_t nameID);

/**
 * @brief Sets a texture value by name.
 * @remark errno will be set on failure.
 * @param values The shared material values.
 * @param name The name of the texture.
 * @param texture The texture to set.
 * @return False if the parameters are invalid, there isn't space available, or a value with the
 *     name is set that isn't a texture.
 */
DS_RENDER_EXPORT bool dsSharedMaterialValues_setTextureName(dsSharedMaterialValues* values,
	const char* name, dsTexture* texture);

/**
 * @brief Sets a texture value by ID.
 * @remark errno will be set on failure.
 * @param values The shared material values.
 * @param nameID The hash of the texture name.
 * @param texture The texture to set.
 * @return False if the parameters are invalid, there isn't space available, or a value with the
 *     name is set that isn't a texture.
 */
DS_RENDER_EXPORT bool dsSharedMaterialValues_setTextureID(dsSharedMaterialValues* values,
	uint32_t nameID, dsTexture* texture);

/**
 * @brief Gets a texture buffer value by name.
 * @param[out] outFormat The texture format to interpret the buffer data. This may be NULL.
 * @param[out] outOffset The offset into the buffer. This may be NULL.
 * @param[out] outCount The number of texels for the buffer. This may be NULL.
 * @param values The shared material values.
 * @param name The name of the buffer.
 * @return The buffer, or NULL if not found or unset.
 */
DS_RENDER_EXPORT dsGfxBuffer* dsSharedMaterialValues_getTextureBufferName(dsGfxFormat* outFormat,
	size_t* outOffset, size_t* outCount, const dsSharedMaterialValues* values, const char* name);

/**
 * @brief Gets a texture buffer value by ID.
 * @param[out] outFormat The texture format to interpret the buffer data. This may be NULL.
 * @param[out] outOffset The offset into the buffer. This may be NULL.
 * @param[out] outCount The number of texels for the buffer. This may be NULL.
 * @param values The shared material values.
 * @param nameID The hash of the buffer name.
 * @return The buffer, or NULL if not found or unset.
 */
DS_RENDER_EXPORT dsGfxBuffer* dsSharedMaterialValues_getTextureBufferID(dsGfxFormat* outFormat,
	size_t* outOffset, size_t* outCount, const dsSharedMaterialValues* values, uint32_t nameID);

/**
 * @brief Sets a texture buffer value by name.
 * @remark errno will be set on failure.
 * @param values The shared material values.
 * @param name The name of the buffer.
 * @param buffer The buffer to set.
 * @param format The texture format to interpret the buffer data.
 * @param offset The offset into the buffer.
 * @param count The number of texels for the buffer.
 * @return False if the parameters are invalid, there isn't space available, or a value with the
 *     name is set that isn't a buffer.
 */
DS_RENDER_EXPORT bool dsSharedMaterialValues_setTextureBufferName(
	dsSharedMaterialValues* values, const char* name, dsGfxBuffer* buffer, dsGfxFormat format,
	size_t offset, size_t count);

/**
 * @brief Sets a texture buffer value by ID.
 * @remark errno will be set on failure.
 * @param values The shared material values.
 * @param nameID The hash of the buffer name.
 * @param buffer The buffer to set.
 * @param format The texture format to interpret the buffer data.
 * @param offset The offset into the buffer.
 * @param count The number of texels for the buffer.
 * @return False if the parameters are invalid, there isn't space available, or a value with the
 *     name is set that isn't a buffer.
 */
DS_RENDER_EXPORT bool dsSharedMaterialValues_setTextureBufferID(dsSharedMaterialValues* values,
	uint32_t nameID, dsGfxBuffer* buffer, dsGfxFormat format, size_t offset, size_t count);

/**
 * @brief Gets a shader variable group value by name.
 * @param values The shared material values.
 * @param name The name of the shader variable group.
 * @return The shader variable group, or NULL if not found or unset.
 */
DS_RENDER_EXPORT dsShaderVariableGroup* dsSharedMaterialValues_getVariableGroupName(
	const dsSharedMaterialValues* values, const char* name);

/**
 * @brief Gets a shader variable group value by ID.
 * @param values The shared material values.
 * @param nameID The hash of the shader variable group name.
 * @return The shader variable group, or NULL if not found or unset.
 */
DS_RENDER_EXPORT dsShaderVariableGroup* dsSharedMaterialValues_getVariableGroupID(
	const dsSharedMaterialValues* values, uint32_t nameID);

/**
 * @brief Sets a shader variable group value by name.
 * @remark errno will be set on failure.
 * @param values The shared material values.
 * @param name The name of the shader variable group.
 * @param group The shader variable group to set.
 * @return False if the parameters are invalid, there isn't space available, or a value with the
 *     name is set that isn't a shader variable group.
 */
DS_RENDER_EXPORT bool dsSharedMaterialValues_setVariableGroupName(
	dsSharedMaterialValues* values, const char* name, dsShaderVariableGroup* group);

/**
 * @brief Sets a shader variable group value by ID.
 * @remark errno will be set on failure.
 * @param values The shared material values.
 * @param nameID The hash of the shader variable group name.
 * @param group The shader variable group to set.
 * @return False if the parameters are invalid, there isn't space available, or a value with the
 *     name is set that isn't a shader variable group.
 */
DS_RENDER_EXPORT bool dsSharedMaterialValues_setVariableGroupID(dsSharedMaterialValues* values,
	uint32_t nameID, dsShaderVariableGroup* group);

/**
 * @brief Gets a buffer value by name.
 * @remark This may also be used for shader variable groups for devices that support uniform block
 *     buffers.
 * @param[out] outOffset The offset into the buffer. This may be NULL.
 * @param[out] outSize The size to use within the buffer. This may be NULL.
 * @param values The shared material values.
 * @param name The name of the buffer.
 * @return The buffer, or NULL if not found or unset.
 */
DS_RENDER_EXPORT dsGfxBuffer* dsSharedMaterialValues_getBufferName(size_t* outOffset,
	size_t* outSize, const dsSharedMaterialValues* values, const char* name);

/**
 * @brief Gets a buffer value by ID.
 * @remark This may also be used for shader variable groups for devices that support uniform block
 *     buffers.
 * @param[out] outOffset The offset into the buffer. This may be NULL.
 * @param[out] outSize The size to use within the buffer. This may be NULL.
 * @param values The shared material values.
 * @param nameID The hash of the buffer name.
 * @return The buffer, or NULL if not found or unset.
 */
DS_RENDER_EXPORT dsGfxBuffer* dsSharedMaterialValues_getBufferID(size_t* outOffset,
	size_t* outSize, const dsSharedMaterialValues* values, uint32_t nameID);

/**
 * @brief Sets a buffer value by name.
 * @remark This may also be used for shader variable groups for devices that support uniform block
 *     buffers.
 * @remark errno will be set on failure.
 * @param values The shared material values.
 * @param name The name of the buffer.
 * @param buffer The buffer to set.
 * @param offset The offset into the buffer.
 * @param size The size to use within the buffer.
 * @return False if the parameters are invalid, there isn't space available, or a value with the
 *     name is set that isn't a buffer.
 */
DS_RENDER_EXPORT bool dsSharedMaterialValues_setBufferName(dsSharedMaterialValues* values,
	const char* name, dsGfxBuffer* buffer, size_t offset, size_t size);

/**
 * @brief Sets a buffer value by ID.
 * @remark This may also be used for shader variable groups for devices that support uniform block
 *     buffers.
 * @remark errno will be set on failure.
 * @param values The shared material values.
 * @param nameID The hash of the buffer name.
 * @param buffer The buffer to set.
 * @param offset The offset into the buffer.
 * @param size The size to use within the buffer.
 * @return False if the parameters are invalid, there isn't space available, or a value with the
 *     name is set that isn't a buffer.
 */
DS_RENDER_EXPORT bool dsSharedMaterialValues_setBufferID(dsSharedMaterialValues* values,
	uint32_t nameID, dsGfxBuffer* buffer, size_t offset, size_t size);

/**
 * @brief Removes a shared material value by name.
 * @param values The shared material values.
 * @param name The name of the value to remvoe.
 * @return True if the value was removed.
 */
DS_RENDER_EXPORT bool dsSharedMaterialValues_removeValueName(dsSharedMaterialValues* values,
	const char* name);

/**
 * @brief Removes a shared material value by ID.
 * @param values The shared material values.
 * @param nameID The hash of the name of the value to remvoe.
 * @return True if the value was removed.
 */
DS_RENDER_EXPORT bool dsSharedMaterialValues_removeValueID(dsSharedMaterialValues* values,
	uint32_t nameID);

/**
 * @brief Clears the shared material values.
 * @remark errno will be set on failure.
 * @param values The shared material values to clear.
 * @return False if values is NULL.
 */
DS_RENDER_EXPORT bool dsSharedMaterialValues_clear(dsSharedMaterialValues* values);

/**
 * @brief Destroys a shared material values instance.
 * @param values The shared material values to destroy.
 */
DS_RENDER_EXPORT void dsSharedMaterialValues_destroy(dsSharedMaterialValues* values);

#ifdef __cplusplus
}
#endif
