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
#include <DeepSea/SceneParticle/Export.h>
#include <DeepSea/SceneParticle/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating scene particle nodes.
 * @see dsSceneParticleNode
 */

/**
 * @brief The type name for a particle node.
 */
DS_SCENEPARTICLE_EXPORT extern const char* const dsSceneParticleNode_typeName;

/**
 * @brief Gets the type of a particle node.
 * @return The type of a particle node.
 */
DS_SCENEPARTICLE_EXPORT const dsSceneNodeType* dsSceneParticleNode_type(void);

/**
 * @brief Creates a scene particle node.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the node with. This must support freeing memory.
 * @param emitterAllocator The allocator to create particle emitters with. The allocator for the
 *     node will be used if this is NULL.
 * @param createEmitterFunc Function to create the particle emitter with.
 * @param userData User data used to aid in creating particle emitters.
 * @param destroyUserDataFunc Function to destroy the user data.
 * @param itemLists The list of item list names that will be used to draw and process the node.
 *     These will be copied.
 * @param itemListCount The number of item lists.
 * @return The scene particle node or NULL if an error occurred.
 */
DS_SCENEPARTICLE_EXPORT dsSceneParticleNode* dsSceneParticleNode_create(dsAllocator* allocator,
	dsAllocator* emitterAllocator, dsCreateSceneParticleNodeEmitterFunction createEmitterFunc,
	void* userData, dsDestroySceneUserDataFunction destroyUserDataFunc,
	const char* const* itemLists, uint32_t itemListCount);

/**
 * @brief Creates a particle emitter for a particle node.
 * @remark errno will be set on failure.
 * @param node The particle node to create an emitter for.
 * @return The emitter or NULL if an error occurred.
 */
DS_SCENEPARTICLE_EXPORT dsParticleEmitter* dsSceneParticleNode_createEmitter(
	const dsSceneParticleNode* node);

#ifdef __cplusplus
}
#endif
