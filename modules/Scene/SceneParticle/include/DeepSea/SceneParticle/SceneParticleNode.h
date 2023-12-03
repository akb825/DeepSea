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
 * @brief Sets up the parent type for a node type subclassing from dsSceneParticleNode.
 * @param type The subclass type for dsSceneParticleNode.
 * @return The type parameter or the type for dsSceneParticleNode if type is NULL.
 */
DS_SCENEPARTICLE_EXPORT const dsSceneNodeType* dsSceneParticleNode_setupParentType(
	dsSceneNodeType* type);

/**
 * @brief Creates a scene particle node.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the node with. This must support freeing memory.
 * @param emitterAllocator The allocator to create particle emitters with. The allocator for the
 *     node will be used if this is NULL.
 * @param createEmitterFunc Function to create a particle emitter with.
 * @param updateEmitterFunc Function to update a particle emitter with. This may be NULL if the
 *     only the standard update via dsParticleEmitter_update() is needed.
 * @param userData User data used to aid in creating particle emitters.
 * @param destroyUserDataFunc Function to destroy the user data. This may be NULL if the user data
 *     doesn't need to be destroyed. This will be called if an error occurs when creating the node.
 * @param itemLists The list of item list names that will be used to draw and process the node.
 *     These will be copied.
 * @param itemListCount The number of item lists.
 * @return The scene particle node or NULL if an error occurred.
 */
DS_SCENEPARTICLE_EXPORT dsSceneParticleNode* dsSceneParticleNode_create(dsAllocator* allocator,
	dsAllocator* emitterAllocator, dsCreateSceneParticleNodeEmitterFunction createEmitterFunc,
	dsUpdateSceneParticleNodeEmitterFunction updateEmitterFunc, void* userData,
	dsDestroyUserDataFunction destroyUserDataFunc, const char* const* itemLists,
	uint32_t itemListCount);

/**
 * @brief Creates a particle emitter for a particle node.
 * @remark errno will be set on failure.
 * @param node The particle node to create an emitter for.
 * @param treeNode The scene tree node the particle emitter will be associated with.
 * @return The emitter or NULL if an error occurred.
 */
DS_SCENEPARTICLE_EXPORT dsParticleEmitter* dsSceneParticleNode_createEmitter(
	const dsSceneParticleNode* node, const dsSceneTreeNode* treeNode);

/**
 * @brief Gets the particle emitter for a tree node.
 * @remark This assumes that the particle emitter was created from a dsSceneParticlePrepare.
 * @param treeNode The tree node to get the particle emitter for.
 * @return The particle emitter or NULL if there isn't one present.
 */
DS_SCENEPARTICLE_EXPORT dsParticleEmitter* dsSceneParticleNode_getEmitterForInstance(
	const dsSceneTreeNode* treeNode);

/**
 * @brief Updates a particle emitter for a particle node.
 * @remark errno will be set on failure.
 * @param node The particle node the emitter was created with.
 * @param emitter The particle emitter to update.
 * @param treeNode The scene tree node the particle emitter is associated with.
 * @param time The time since the last update in seconds.
 * @return False if the emitter couldn't be updated.
 */
DS_SCENEPARTICLE_EXPORT bool dsSceneParticleNode_updateEmitter(const dsSceneParticleNode* node,
	dsParticleEmitter* emitter, const dsSceneTreeNode* treeNode, float time);

#ifdef __cplusplus
}
#endif
