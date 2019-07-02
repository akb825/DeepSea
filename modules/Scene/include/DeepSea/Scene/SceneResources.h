/*
 * Copyright 2019 Aaron Barany
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
#include <DeepSea/Render/Types.h>
#include <DeepSea/Scene/Export.h>
#include <DeepSea/Scene/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating scene resource collections.
 * @see dsSceneResources
 */

/**
 * @brief Gets the size of dsSceneResources.
 * @return sizeof(dsSceneResources)
 */
DS_SCENE_EXPORT size_t dsSceneResources_sizeof(void);

/**
 * @brief Gets the full allocated size of dsSceneResources.
 * @param maxTextures The maximum number of textures that can be held.
 * @param maxBuffers The maximum number of buffers that can be held.
 * @param maxShaderModules The maximum number of shader modules that can be held.
 * @param maxShaders The maximum number of shaders that can be held.
 * @param maxDrawGeometries The maximum number of draw geometries that can be held.
 * @return The full allocated size of the dsSceneResources instance.
 */
DS_SCENE_EXPORT size_t dsSceneResources_fullAllocSize(uint32_t maxTextures, uint32_t maxBuffers,
	uint32_t maxShaderModules, uint32_t maxShaders, uint32_t maxDrawGeometries);

/**
 * @brief Creates a scene resources object.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the scene resources with.
 * @param maxTextures The maximum number of textures that can be held.
 * @param maxBuffers The maximum number of buffers that can be held.
 * @param maxShaderModules The maximum number of shader modules that can be held.
 * @param maxShaders The maximum number of shaders that can be held.
 * @param maxDrawGeometries The maximum number of draw geometries that can be held.
 * @return The scene resources or NULL if an error occurred..
 */
DS_SCENE_EXPORT dsSceneResources* dsSceneResources_create(dsAllocator* allocator,
	uint32_t maxTextures, uint32_t maxBuffers, uint32_t maxShaderModules, uint32_t maxShaders,
	uint32_t maxDrawGeometries);

/**
 * @brief Gets the number of remaining textures that can be set.
 * @param resources The scene resources.
 * @return The number of remaining textures.
 */
DS_SCENE_EXPORT uint32_t dsSceneResources_getRemainingTextures(const dsSceneResources* resources);

/**
 * @brief Adds a texture to the scene resources.
 * @remark errno will be set on failure.
 * @param resources The scene resources.
 * @param name The name of the texture. The length, including the null terminator, must not exceed
 *     DS_MAX_SCENE_RESOURCE_NAME_LENGTH.
 * @param texture The texture to add.
 * @param own True to take ownership of the texture.
 * @return False if the texture couldn't be added.
 */
DS_SCENE_EXPORT bool dsSceneResources_addTexture(dsSceneResources* resources,
	const char* name, dsTexture* texture, bool own);

/**
 * @brief Removes a texture from the scene resources.
 * @remark errno will be set on failure.
 * @param resources The scene resources.
 * @param name The name of the texture to remove.
 * @param relinquish True to relinquish ownership of the texture. This will prevent this from
 *     freeing the texture if it is owned.
 * @return False if the texture couldn't be removed.
 */
DS_SCENE_EXPORT bool dsSceneResource_removeTexture(dsSceneResources* resources,
	const char* name, bool relinquish);

/**
 * @brief Finds a texture in the scene resources.
 * @param resources The scene resources.
 * @param name The name of the texture.
 * @return The texture or NULL if it couldn't be found.
 */
DS_SCENE_EXPORT dsTexture* dsSceneResources_findTexture(const dsSceneResources* resources,
	const char* name);

/**
 * @brief Gets the number of remaining buffers that can be set.
 * @param resources The scene resources.
 * @return The number of remaining buffers.
 */
DS_SCENE_EXPORT uint32_t dsSceneResources_getRemainingBuffers(const dsSceneResources* resources);

/**
 * @brief Adds a buffer to the scene resources.
 * @remark errno will be set on failure.
 * @param resources The scene resources.
 * @param name The name of the buffer. The length, including the null terminator, must not exceed
 *     DS_MAX_SCENE_RESOURCE_NAME_LENGTH.
 * @param buffer The buffer to add.
 * @param own True to take ownership of the buffer.
 * @return False if the buffer couldn't be added.
 */
DS_SCENE_EXPORT bool dsSceneResources_addBuffer(dsSceneResources* resources,
	const char* name, dsGfxBuffer* buffer, bool own);

/**
 * @brief Removes a buffer from the scene resources.
 * @remark errno will be set on failure.
 * @param resources The scene resources.
 * @param name The name of the buffer to remove.
 * @param relinquish True to relinquish ownership of the buffer. This will prevent this from
 *     freeing the buffer if it is owned.
 * @return False if the buffer couldn't be removed.
 */
DS_SCENE_EXPORT bool dsSceneResource_removeBuffer(dsSceneResources* resources,
	const char* name, bool relinquish);

/**
 * @brief Finds a buffer in the scene resources.
 * @param resources The scene resources.
 * @param name The name of the buffer.
 * @return The buffer or NULL if it couldn't be found.
 */
DS_SCENE_EXPORT dsGfxBuffer* dsSceneResources_findBuffer(const dsSceneResources* resources,
	const char* name);

/**
 * @brief Gets the number of remaining shader modules that can be set.
 * @param resources The scene resources.
 * @return The number of remaining shader modules.
 */
DS_SCENE_EXPORT uint32_t dsSceneResources_getRemainingShaderModules(
	const dsSceneResources* resources);

/**
 * @brief Adds a shader module to the scene resources.
 * @remark errno will be set on failure.
 * @param resources The scene resources.
 * @param name The name of the shader module. The length, including the null terminator, must not
 *     exceed DS_MAX_SCENE_RESOURCE_NAME_LENGTH.
 * @param shaderModule The shader module to add.
 * @param own True to take ownership of the shader module.
 * @return False if the shader module couldn't be added.
 */
DS_SCENE_EXPORT bool dsSceneResources_addShaderModule(dsSceneResources* resources,
	const char* name, dsShaderModule* shaderModule, bool own);

/**
 * @brief Removes a shader module from the scene resources.
 * @remark errno will be set on failure.
 * @param resources The scene resources.
 * @param name The name of the shader module to remove.
 * @param relinquish True to relinquish ownership of the shader module. This will prevent this from
 *     freeing the shader module if it is owned.
 * @return False if the shader module couldn't be removed.
 */
DS_SCENE_EXPORT bool dsSceneResource_removeShaderModule(dsSceneResources* resources,
	const char* name, bool relinquish);

/**
 * @brief Finds a shader module in the scene resources.
 * @param resources The scene resources.
 * @param name The name of the shader module.
 * @return The shader module or NULL if it couldn't be found.
 */
DS_SCENE_EXPORT dsShaderModule* dsSceneResources_findShaderModule(const dsSceneResources* resources,
	const char* name);

/**
 * @brief Gets the number of remaining shaders that can be set.
 * @param resources The scene resources.
 * @return The number of remaining shaders.
 */
DS_SCENE_EXPORT uint32_t dsSceneResources_getRemainingShaders(const dsSceneResources* resources);

/**
 * @brief Adds a shader to the scene resources.
 * @remark errno will be set on failure.
 * @param resources The scene resources.
 * @param name The name of the shader. The length, including the null terminator, must not exceed
 *     DS_MAX_SCENE_RESOURCE_NAME_LENGTH.
 * @param shader The shader to add.
 * @param own True to take ownership of the shader.
 * @return False if the shader couldn't be added.
 */
DS_SCENE_EXPORT bool dsSceneResources_addShader(dsSceneResources* resources,
	const char* name, dsShader* shader, bool own);

/**
 * @brief Removes a shader from the scene resources.
 * @remark errno will be set on failure.
 * @param resources The scene resources.
 * @param name The name of the shader to remove.
 * @param relinquish True to relinquish ownership of the shader. This will prevent this from
 *     freeing the shader if it is owned.
 * @return False if the shader couldn't be removed.
 */
DS_SCENE_EXPORT bool dsSceneResource_removeShader(dsSceneResources* resources,
	const char* name, bool relinquish);

/**
 * @brief Finds a shader in the scene resources.
 * @param resources The scene resources.
 * @param name The name of the shader.
 * @return The shader or NULL if it couldn't be found.
 */
DS_SCENE_EXPORT dsShader* dsSceneResources_findShader(const dsSceneResources* resources,
	const char* name);

/**
 * @brief Gets the number of remaining draw geometries that can be set.
 * @param resources The scene resources.
 * @return The number of remaining draw geometries.
 */
DS_SCENE_EXPORT uint32_t dsSceneResources_getRemainingDrawGeometries(
	const dsSceneResources* resources);

/**
 * @brief Adds a draw geometry to the scene resources.
 * @remark errno will be set on failure.
 * @param resources The scene resources.
 * @param name The name of the draw geometry. The length, including the null terminator, must not
 *     exceed DS_MAX_SCENE_RESOURCE_NAME_LENGTH.
 * @param drawGeometry The draw geometry to add.
 * @param own True to take ownership of the draw geometry.
 * @return False if the draw geometry couldn't be added.
 */
DS_SCENE_EXPORT bool dsSceneResources_addDrawGeometry(dsSceneResources* resources,
	const char* name, dsDrawGeometry* drawGeometry, bool own);

/**
 * @brief Removes a draw geometry from the scene resources.
 * @remark errno will be set on failure.
 * @param resources The scene resources.
 * @param name The name of the draw geometry to remove.
 * @param relinquish True to relinquish ownership of the draw geometry. This will prevent this from
 *     freeing the draw geometry if it is owned.
 * @return False if the draw geometry couldn't be removed.
 */
DS_SCENE_EXPORT bool dsSceneResource_removeDrawGeometry(dsSceneResources* resources,
	const char* name, bool relinquish);

/**
 * @brief Finds a draw geometry in the scene resources.
 * @param resources The scene resources.
 * @param name The name of the draw geometry.
 * @return The draw geometry or NULL if it couldn't be found.
 */
DS_SCENE_EXPORT dsDrawGeometry* dsSceneResources_findDrawGeometry(const dsSceneResources* resources,
	const char* name);

/**
 * @brief Adds the reference count for the scene resources.
 * @remark This function is thread-safe.
 * @param resources The resources to add a reference count to.
 * @return The resources with an incremented reference count.
 */
DS_SCENE_EXPORT dsSceneResources* dsSceneResources_addRef(dsSceneResources* resources);

/**
 * @brief Subtracts the reference count to the node.
 *
 * Once the reference count reaches 0 the resources will be destroyed.
 *
 * @remark This function is thread-safe, though should only be called from threads with an active
 *     resource context if there are owned resources.
 * @param resources The resources to subtract the reference from.
 */
DS_SCENE_EXPORT void dsSceneResources_freeRef(dsSceneResources* resources);

#ifdef __cplusplus
}
#endif
