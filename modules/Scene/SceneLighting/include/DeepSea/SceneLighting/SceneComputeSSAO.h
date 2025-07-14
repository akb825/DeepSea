/*
 * Copyright 2022 Aaron Barany
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
#include <DeepSea/Scene/Types.h>
#include <DeepSea/SceneLighting/Export.h>
#include <DeepSea/SceneLighting/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating scene compute screen-space ambient occlusion.
 * @see dsSceneComputeSSAO
 */

/**
 * @brief The size of a tile (in X and Y) for the compute shader when dispatching SSAO.
 */
#define DS_SCENE_COMPUTE_SSAO_TILE_SIZE 16

/**
 * @brief The scene compute SSAO type name.
 */
DS_SCENELIGHTING_EXPORT extern const char* const dsSceneComputeSSAO_typeName;

/**
 * @brief Gets the type of a scene compute SSAO.
 * @return The type of a scene compute SSAO.
 */
DS_SCENELIGHTING_EXPORT const dsSceneItemListType* dsSceneComputeSSAO_type(void);

/**
 * @brief Creates a scene compute screen-space ambient occlusion.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the scene SSAO with. This must support freeing memory.
 * @param resourceManager The resource manager to create graphics resources with.
 * @param resourceAllocator The allocator to create graphics resources with. If NULL this will use
 *     the scene SSAO allocator.
 * @param name The name of the scene SSAO. This will be copied.
 * @param shader The shader used to compute SSAO.
 * @param material The material for the SSAO. This must have the following two elements with
 *     material binding:
 *     - RandomOffsets: Uniform block buffer with a single array of DS_MAX_SCENE_SSAO_SAMPLES vec3
 *       elements. This should be multiplied by the radius for the final offset.
 *     - randomRotations: 2D RG texture for a random rotation vector to cross with the normal. The
 *       Z coordinate is implicitly 0. This is of size DS_SCENE_SSAO_ROTATION_SIZE.
 * @return The scene SSAO or NULL if an error occurred.
 */
DS_SCENELIGHTING_EXPORT dsSceneComputeSSAO* dsSceneComputeSSAO_create(dsAllocator* allocator,
	dsResourceManager* resourceManager, dsAllocator* resourceAllocator, const char* name,
	dsShader* shader, dsMaterial* material);

/**
 * @brief Gets the shader.
 * @param ssao The scene SSAO.
 * @return The shader or NULL if ssao is NULL.
 */
DS_SCENELIGHTING_EXPORT dsShader* dsSceneComputeSSAO_getShader(const dsSceneComputeSSAO* ssao);

/**
 * @brief Sets the shader.
 * @remark errno will be set on failure.
 * @param ssao The scene SSAO.
 * @param shader The shader. The vertex elements for the shader are:
 *     - position: vec2 clip-space [-1, 1] values.
 * @return False if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT bool dsSceneComputeSSAO_setShader(dsSceneComputeSSAO* ssao, dsShader* shader);

/**
 * @brief Gets the material.
 * @param ssao The scene SSAO.
 * @return The material or NULL if ssao is NULL.
 */
DS_SCENELIGHTING_EXPORT dsMaterial* dsSceneComputeSSAO_getMaterial(const dsSceneComputeSSAO* ssao);

/**
 * @brief Sets the material.
 * @remark errno will be set on failure.
 * @param ssao The scene SSAO.
 * @param material The material. This must have the following two elements with material binding:
 *     - RandomOffsets: Uniform block buffer with a single array of DS_MAX_SCENE_SSAO_SAMPLES vec3
 *       elements.
 *     - randomRotations: 2D RG texture for a random rotation vector to cross with the normal.
 * @return False if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT bool dsSceneComputeSSAO_setMaterial(dsSceneComputeSSAO* ssao, dsMaterial* material);

/**
 * @brief Destroys a scene SSAO.
 * @param ssao The scene SSAO to destroy.
 */
DS_SCENELIGHTING_EXPORT void dsSceneComputeSSAO_destroy(dsSceneComputeSSAO* ssao);

#ifdef __cplusplus
}
#endif


