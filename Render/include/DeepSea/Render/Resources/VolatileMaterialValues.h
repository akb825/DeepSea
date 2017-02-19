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
DS_RENDER_EXPORT dsTexture* dsVolatileMaterialValue_getTextureName(
	const dsVolatileMaterialValues* values, const char* name);

/**
 * @brief Gets a texture value by ID.
 * @param values The volatile material values.
 * @param nameId The hash of the texture name.
 * @return The texture, or NULL if not found or unset.
 */
DS_RENDER_EXPORT dsTexture* dsVolatileMaterialValue_getTextureId(
	const dsVolatileMaterialValues* values, uint32_t nameId);

/**
 * @brief Sets a texture value by name.
 * @param values The volatile material values.
 * @param name The name of the texture.
 * @param texture The texture to set.
 * @return False if the parameters are invalid, there isn't space available, or a value with the
 *     name is set that isn't a texture.
 */
DS_RENDER_EXPORT bool dsVolatileMaterialValue_setTextureName(const dsVolatileMaterialValues* values,
	const char* name, dsTexture* texture);

/**
 * @brief Sets a texture value by ID.
 * @param values The volatile material values.
 * @param nameId The hash of the texture name.
 * @param texture The texture to set.
 * @return False if the parameters are invalid, there isn't space available, or a value with the
 *     name is set that isn't a texture.
 */
DS_RENDER_EXPORT bool dsVolatileMaterialValue_setTextureId(const dsVolatileMaterialValues* values,
	uint32_t nameId, dsTexture* texture);

/**
 * @brief Gets a shader variable group value by name.
 * @param values The volatile material values.
 * @param name The name of the shader variable group.
 * @return The shader variable group, or NULL if not found or unset.
 */
DS_RENDER_EXPORT dsShaderVariableGroup* dsVolatileMaterialValue_getVariableGroupName(
	const dsVolatileMaterialValues* values, const char* name);

/**
 * @brief Gets a shader variable group value by ID.
 * @param values The volatile material values.
 * @param nameId The hash of the shader variable group name.
 * @return The shader variable group, or NULL if not found or unset.
 */
DS_RENDER_EXPORT dsShaderVariableGroup* dsVolatileMaterialValue_getVariableGroupId(
	const dsVolatileMaterialValues* values, uint32_t nameId);

/**
 * @brief Sets a shader variable group value by name.
 * @param values The volatile material values.
 * @param name The name of the shader variable group.
 * @param group The shader variable group to set.
 * @return False if the parameters are invalid, there isn't space available, or a value with the
 *     name is set that isn't a shader variable group.
 */
DS_RENDER_EXPORT bool dsVolatileMaterialValue_setVariableGroupName(
	const dsVolatileMaterialValues* values, const char* name, dsShaderVariableGroup* group);

/**
 * @brief Sets a shader variable group value by ID.
 * @param values The volatile material values.
 * @param nameId The hash of the shader variable group name.
 * @param group The shader variable group to set.
 * @return False if the parameters are invalid, there isn't space available, or a value with the
 *     name is set that isn't a shader variable group.
 */
DS_RENDER_EXPORT bool dsVolatileMaterialValue_setVariableGroupId(
	const dsVolatileMaterialValues* values, uint32_t nameId, dsShaderVariableGroup* group);

/**
 * @brief Gets a buffer value by name.
 * @param values The volatile material values.
 * @param name The name of the buffer.
 * @return The buffer, or NULL if not found or unset.
 */
DS_RENDER_EXPORT dsGfxBuffer* dsVolatileMaterialValue_getBufferName(
	const dsVolatileMaterialValues* values, const char* name);

/**
 * @brief Gets a buffer value by ID.
 * @param values The volatile material values.
 * @param nameId The hash of the buffer name.
 * @return The buffer, or NULL if not found or unset.
 */
DS_RENDER_EXPORT dsGfxBuffer* dsVolatileMaterialValue_getBufferId(
	const dsVolatileMaterialValues* values, uint32_t nameId);

/**
 * @brief Sets a buffer value by name.
 * @param values The volatile material values.
 * @param name The name of the buffer.
 * @param buffer The buffer to set.
 * @return False if the parameters are invalid, there isn't space available, or a value with the
 *     name is set that isn't a buffer.
 */
DS_RENDER_EXPORT bool dsVolatileMaterialValue_setBufferName(const dsVolatileMaterialValues* values,
	const char* name, dsGfxBuffer* buffer);

/**
 * @brief Sets a buffer value by ID.
 * @param values The volatile material values.
 * @param nameId The hash of the buffer name.
 * @param buffer The buffer to set.
 * @return False if the parameters are invalid, there isn't space available, or a value with the
 *     name is set that isn't a buffer.
 */
DS_RENDER_EXPORT bool dsVolatileMaterialValue_setBufferId(const dsVolatileMaterialValues* values,
	uint32_t nameId, dsGfxBuffer* buffer);

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
