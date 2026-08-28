/*
 * Copyright 2023-2026 Aaron Barany
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
#include <DeepSea/SceneAnimation/Export.h>
#include <DeepSea/SceneAnimation/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions to create scene skinning data.
 */

/**
 * @brief The scene skinning data type name.
 */
DS_SCENEANIMATION_EXPORT extern const char* const dsSceneSkinningData_typeName;

/**
 * @brief The scene skinning data shader uniform name.
 */
DS_SCENEANIMATION_EXPORT extern const char* const dsSceneSkinningData_uniformName;

/**
 * @brief The scene skinning data shader uniform name for texture data.
 */
DS_SCENEANIMATION_EXPORT extern const char* const dsSceneSkinningData_textureDataUniformName;

/**
 * @brief Gets the type of scene skinning data.
 * @return The type of scene skinning data.
 */
DS_SCENEANIMATION_EXPORT const dsSceneInstanceDataType* dsSceneSkinningData_type(void);

/**
 * @brief Checks for the expected material type based on host support.
 * @param resourceManager The resource manager.
 * @return The material type, as one of either dsMaterialType_UniformBuffer,
 *     dsMaterialType_TextureBuffer, or dsMaterialType_Texture.
 */
DS_SCENEANIMATION_EXPORT dsMaterialType dsSceneSkinningData_materialType(
	const dsResourceManager* resourceManager);

/**
 * @brief Creates the shader variable group description used for texture skinning info.
 *
 * This is only valid to call if dsSceneSkinningData_materialType() returns either
 * dsMaterialType_TextureBuffer or dsMaterialType_Texture;
 *
 * @remark This should be shared among all dsSceneSkinningData instances.
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager.
 * @param allocator The allocator to create the shader variable group with. If NULL, the allocator
 *     from resourceManager.
 * @return The shader variable group description or NULL if an error occurred.
 */
DS_SCENEANIMATION_EXPORT dsShaderVariableGroupDesc*
	dsSceneSkinningData_createTextureInfoShaderVariableGroupDesc(
		dsResourceManager* resourceManager, dsAllocator* allocator);

/**
 * @brief Checks whether or not a shader variable group is compatible with texture info for
 *     dsSceneSkinningData.
 * @param resourceManager The resource manager.
 * @param textureInfoDesc The shader variable group for the transform. This is expected to be NULL
 *     if dsSceneSkinningData_materialType() returns dsMaterialType_UniformBuffer().
 * @return Whether or not transformDesc is compatible.
 */
DS_SCENEANIMATION_EXPORT bool dsSceneSkinningData_isTextureInfoShaderVariableGroupCompatible(
	const dsResourceManager* resourceManager, const dsShaderVariableGroupDesc* textureInfoDesc);

/**
 * @brief Creates a scene skinning data to use with a dsSceneItemList.
 * @param resourceManager The resource manager.
 * @param allocator The allocator to create the skinning data with. This must support freeing
 *     memory.
 * @param resourceAllocator The allocator for graphics resources. Defaults to allocator if NULL.
 * @param textureInfoDesc The shader variable group description created from
 *     dsViewTransformData_createShaderVariableGroupDesc(), or NULL if
 *     dsSceneSkinningData_materialType() returns dsMaterialType_UniformBuffer. This must remain
 *     alive at least as long as the instance data object.
 * @return The scene skinning data or NULL if an error occurred.
 */
DS_SCENEANIMATION_EXPORT dsSceneInstanceData* dsSceneSkinningData_create(
	dsAllocator* allocator, dsResourceManager* resourceManager, dsAllocator* resourceAllocator,
	const dsShaderVariableGroupDesc* textureInfoDesc);

#ifdef __cplusplus
}
#endif
