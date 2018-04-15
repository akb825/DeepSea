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
 * @brief Functions for creating and using volatile material values.
 *
 * This holds a set of values to be used for volatile material elements indexed by name. Volatile
 * material values are texture, image, and buffer material values declared as volatile within
 * dsMaterialDesc. This allows values for the current rendering state to be stored separately from
 * the material properties. Separate instances can be used for different render passes, draw
 * threads, etc. to remain independent between multible uses of the material, and the values stored
 * in this may be changed in-between draw calls.
 *
 * Lookups into this will be frequent, so as a result the index is done by pre-hashing the name. You
 * may either access the elements by name or by the ID, which is the hash of the name. (by calling
 * dsHashString())
 *
 * This will prevent setting a value when a value is set with the same name but a different type.
 * In this case, errno will be set to EPERM. errno will be set to ENOMEM if the maximum number of
 * values is exceeded.
 *
 * @see dsVolatileMaterialValues
 */

/**
 * @brief The default maximum number of volatile material values.
 */
#define DS_DEFAULT_MAX_VOLATILE_MATERIAL_VALUES 100

/**
 * @brief Gets the size of dsVolatileMaterialValues.
 * @return The size of dsVolatileMaterialValues.
 */
DS_RENDER_EXPORT size_t dsVolatileMaterialValues_sizeof(void);

/**
 * @brief Gets the full allocated size of dsVolatileMaterialValues.
 * @param maxValues The maximum number of values that can be stored.
 * @return The full allocated size of dsVolatileMaterialValues.
 */
DS_RENDER_EXPORT size_t dsVolatileMaterialValues_fullAllocSize(unsigned int maxValues);

/**
 * @brief Creates a volatile material values instance.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the volatile material values with.
 * @param maxValues The maximum number of values to use.
 */
DS_RENDER_EXPORT dsVolatileMaterialValues* dsVolatileMaterialValues_create(dsAllocator* allocator,
	unsigned int maxValues);

/**
 * @brief Gets the number of values stored in a volatile material values instance.
 * @param values The volatile material values.
 * @return The number of values.
 */
DS_RENDER_EXPORT unsigned int dsVolatileMaterialValues_getValueCount(
	const dsVolatileMaterialValues* values);

/**
 * @brief Gets the maximum number of values that can be stored in a volatile material values
 *     instance.
 * @param values The volatile material values.
 * @return The maximum number of values.
 */
DS_RENDER_EXPORT unsigned int dsVolatileMaterialValues_getMaxValueCount(
	const dsVolatileMaterialValues* values);

/**
 * @brief Gets a texture value by name.
 * @param values The volatile material values.
 * @param name The name of the texture.
 * @return The texture, or NULL if not found or unset.
 */
DS_RENDER_EXPORT dsTexture* dsVolatileMaterialValues_getTextureName(
	const dsVolatileMaterialValues* values, const char* name);

/**
 * @brief Gets a texture value by ID.
 * @param values The volatile material values.
 * @param nameId The hash of the texture name.
 * @return The texture, or NULL if not found or unset.
 */
DS_RENDER_EXPORT dsTexture* dsVolatileMaterialValues_getTextureId(
	const dsVolatileMaterialValues* values, uint32_t nameId);

/**
 * @brief Sets a texture value by name.
 * @remark errno will be set on failure.
 * @param values The volatile material values.
 * @param name The name of the texture.
 * @param texture The texture to set.
 * @return False if the parameters are invalid, there isn't space available, or a value with the
 *     name is set that isn't a texture.
 */
DS_RENDER_EXPORT bool dsVolatileMaterialValues_setTextureName(dsVolatileMaterialValues* values,
	const char* name, dsTexture* texture);

/**
 * @brief Sets a texture value by ID.
 * @remark errno will be set on failure.
 * @param values The volatile material values.
 * @param nameId The hash of the texture name.
 * @param texture The texture to set.
 * @return False if the parameters are invalid, there isn't space available, or a value with the
 *     name is set that isn't a texture.
 */
DS_RENDER_EXPORT bool dsVolatileMaterialValues_setTextureId(dsVolatileMaterialValues* values,
	uint32_t nameId, dsTexture* texture);

/**
 * @brief Gets a texture buffer value by name.
 * @param[out] outFormat The texture format to interpret the buffer data. This may be NULL.
 * @param[out] outOffset The offset into the buffer. This may be NULL.
 * @param[out] outCount The number of texels for the buffer. This may be NULL.
 * @param values The volatile material values.
 * @param name The name of the buffer.
 * @return The buffer, or NULL if not found or unset.
 */
DS_RENDER_EXPORT dsGfxBuffer* dsVolatileMaterialValues_getTextureBufferName(dsGfxFormat* outFormat,
	size_t* outOffset, size_t* outCount, const dsVolatileMaterialValues* values, const char* name);

/**
 * @brief Gets a texture buffer value by ID.
 * @param[out] outFormat The texture format to interpret the buffer data. This may be NULL.
 * @param[out] outOffset The offset into the buffer. This may be NULL.
 * @param[out] outCount The number of texels for the buffer. This may be NULL.
 * @param values The volatile material values.
 * @param nameId The hash of the buffer name.
 * @return The buffer, or NULL if not found or unset.
 */
DS_RENDER_EXPORT dsGfxBuffer* dsVolatileMaterialValues_getTextureBufferId(dsGfxFormat* outFormat,
	size_t* outOffset, size_t* outCount, const dsVolatileMaterialValues* values, uint32_t nameId);

/**
 * @brief Sets a texture buffer value by name.
 * @remark errno will be set on failure.
 * @param values The volatile material values.
 * @param name The name of the buffer.
 * @param buffer The buffer to set.
 * @param format The texture format to interpret the buffer data.
 * @param offset The offset into the buffer.
 * @param count The number of texels for the buffer.
 * @return False if the parameters are invalid, there isn't space available, or a value with the
 *     name is set that isn't a buffer.
 */
DS_RENDER_EXPORT bool dsVolatileMaterialValues_setTextureBufferName(
	dsVolatileMaterialValues* values, const char* name, dsGfxBuffer* buffer, dsGfxFormat format,
	size_t offset, size_t count);

/**
 * @brief Sets a texture buffer value by ID.
 * @remark errno will be set on failure.
 * @param values The volatile material values.
 * @param nameId The hash of the buffer name.
 * @param buffer The buffer to set.
 * @param format The texture format to interpret the buffer data.
 * @param offset The offset into the buffer.
 * @param count The number of texels for the buffer.
 * @return False if the parameters are invalid, there isn't space available, or a value with the
 *     name is set that isn't a buffer.
 */
DS_RENDER_EXPORT bool dsVolatileMaterialValues_setTextureBufferId(dsVolatileMaterialValues* values,
	uint32_t nameId, dsGfxBuffer* buffer, dsGfxFormat format, size_t offset, size_t count);

/**
 * @brief Gets a shader variable group value by name.
 * @param values The volatile material values.
 * @param name The name of the shader variable group.
 * @return The shader variable group, or NULL if not found or unset.
 */
DS_RENDER_EXPORT dsShaderVariableGroup* dsVolatileMaterialValues_getVariableGroupName(
	const dsVolatileMaterialValues* values, const char* name);

/**
 * @brief Gets a shader variable group value by ID.
 * @param values The volatile material values.
 * @param nameId The hash of the shader variable group name.
 * @return The shader variable group, or NULL if not found or unset.
 */
DS_RENDER_EXPORT dsShaderVariableGroup* dsVolatileMaterialValues_getVariableGroupId(
	const dsVolatileMaterialValues* values, uint32_t nameId);

/**
 * @brief Sets a shader variable group value by name.
 * @remark errno will be set on failure.
 * @param values The volatile material values.
 * @param name The name of the shader variable group.
 * @param group The shader variable group to set.
 * @return False if the parameters are invalid, there isn't space available, or a value with the
 *     name is set that isn't a shader variable group.
 */
DS_RENDER_EXPORT bool dsVolatileMaterialValues_setVariableGroupName(
	dsVolatileMaterialValues* values, const char* name, dsShaderVariableGroup* group);

/**
 * @brief Sets a shader variable group value by ID.
 * @remark errno will be set on failure.
 * @param values The volatile material values.
 * @param nameId The hash of the shader variable group name.
 * @param group The shader variable group to set.
 * @return False if the parameters are invalid, there isn't space available, or a value with the
 *     name is set that isn't a shader variable group.
 */
DS_RENDER_EXPORT bool dsVolatileMaterialValues_setVariableGroupId(dsVolatileMaterialValues* values,
	uint32_t nameId, dsShaderVariableGroup* group);

/**
 * @brief Gets a buffer value by name.
 * @param[out] outOffset The offset into the buffer. This may be NULL.
 * @param[out] outSize The size to use within the buffer. This may be NULL.
 * @param values The volatile material values.
 * @param name The name of the buffer.
 * @return The buffer, or NULL if not found or unset.
 */
DS_RENDER_EXPORT dsGfxBuffer* dsVolatileMaterialValues_getBufferName(size_t* outOffset,
	size_t* outSize, const dsVolatileMaterialValues* values, const char* name);

/**
 * @brief Gets a buffer value by ID.
 * @param[out] outOffset The offset into the buffer. This may be NULL.
 * @param[out] outSize The size to use within the buffer. This may be NULL.
 * @param values The volatile material values.
 * @param nameId The hash of the buffer name.
 * @return The buffer, or NULL if not found or unset.
 */
DS_RENDER_EXPORT dsGfxBuffer* dsVolatileMaterialValues_getBufferId(size_t* outOffset,
	size_t* outSize, const dsVolatileMaterialValues* values, uint32_t nameId);

/**
 * @brief Sets a buffer value by name.
 * @remark errno will be set on failure.
 * @param values The volatile material values.
 * @param name The name of the buffer.
 * @param buffer The buffer to set.
 * @param offset The offset into the buffer.
 * @param size The size to use within the buffer.
 * @return False if the parameters are invalid, there isn't space available, or a value with the
 *     name is set that isn't a buffer.
 */
DS_RENDER_EXPORT bool dsVolatileMaterialValues_setBufferName(dsVolatileMaterialValues* values,
	const char* name, dsGfxBuffer* buffer, size_t offset, size_t size);

/**
 * @brief Sets a buffer value by ID.
 * @remark errno will be set on failure.
 * @param values The volatile material values.
 * @param nameId The hash of the buffer name.
 * @param buffer The buffer to set.
 * @param offset The offset into the buffer.
 * @param size The size to use within the buffer.
 * @return False if the parameters are invalid, there isn't space available, or a value with the
 *     name is set that isn't a buffer.
 */
DS_RENDER_EXPORT bool dsVolatileMaterialValues_setBufferId(dsVolatileMaterialValues* values,
	uint32_t nameId, dsGfxBuffer* buffer, size_t offset, size_t size);

/**
 * @brief Removes a volatile material value by name.
 * @param values The volatile material values.
 * @param name The name of the value to remvoe.
 * @return True if the value was removed.
 */
DS_RENDER_EXPORT bool dsVolatileMaterialValues_removeValueName(dsVolatileMaterialValues* values,
	const char* name);

/**
 * @brief Removes a volatile material value by ID.
 * @param values The volatile material values.
 * @param nameId The hash of the name of the value to remvoe.
 * @return True if the value was removed.
 */
DS_RENDER_EXPORT bool dsVolatileMaterialValues_removeValueId(dsVolatileMaterialValues* values,
	uint32_t nameId);

/**
 * @brief Clears the volatile material values.
 * @remark errno will be set on failure.
 * @param values The volatile material values to clear.
 * @return False if values is NULL.
 */
DS_RENDER_EXPORT bool dsVolatileMaterialValues_clear(dsVolatileMaterialValues* values);

/**
 * @brief Destroys a volatile material values instance.
 * @param values The volatile material values to destroy.
 */
DS_RENDER_EXPORT void dsVolatileMaterialValues_destroy(dsVolatileMaterialValues* values);

#ifdef __cplusplus
}
#endif
