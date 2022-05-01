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
#include <DeepSea/Particle/Export.h>
#include <DeepSea/Particle/Types.h>
#include <DeepSea/Render/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating particle drawers.
 * @see dsParticleDrawer
 */

/**
 * @brief Creates a particle drawer.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the drawer with. This must support freeing memory.
 * @param resourceManager The resource manager to create graphics resources with.
 * @param resourceAllocator The allocator to create graphics resources with. If NULL, allocator
 *     will be used.
 * @return The particle drawer or NULL if an error occurred.
 */
DS_PARTICLE_EXPORT dsParticleDraw* dsParticleDraw_create(dsAllocator* allocator,
	dsResourceManager* resourceManager, dsAllocator* resourceAllocator);

/**
 * @brief Adds a particle emitter to the drawer.
 *
 * Emitters that are destroyed will automatically be removed from the drawer. Likewise, the
 * references to handle automatic removal will be removed if the drawer is destroyed first.
 *
 * @remark errno will be set on failure.
 * @remark This function is thread-safe.
 * @param drawer The particle drawer to add the emitter to.
 * @param emitter The particle emitter to add to the drawer.
 * @return Whether or not the emitter was added.
 */
DS_PARTICLE_EXPORT bool dsParticleDraw_addEmitter(dsParticleDraw* drawer,
	dsParticleEmitter* emitter);

/**
 * @brief Removes a particle emitter to the drawer.
 * @remark errno will be set on failure.
 * @remark This function is thread-safe.
 * @param drawer The particle drawer to remove the emitter from.
 * @param emitter The particle emitter to remove from the drawer.
 * @return Whether or not the emitter was removed..
 */
DS_PARTICLE_EXPORT bool dsParticleDraw_removeEmitter(dsParticleDraw* drawer,
	dsParticleEmitter* emitter);

/**
 * @brief Draws the set of particle emitters that have added to it.
 * @remark errno will be set on failure.
 * @param drawer The particle draw to render the contents of.
 * @param commandBuffer The command buffer to add graphics commands to.
 * @param globalValues The global material values to use with the materials for the particles.
 * @param viewMatrix The view matrix the particles will be drawn with.
 * @param viewFrustum The frustum of the view to cull particle emitters.
 * @return False if an error occurred.
 */
DS_PARTICLE_EXPORT bool dsParticleDraw_draw(dsParticleDraw* drawer,
	dsCommandBuffer* commandBuffer, const dsSharedMaterialValues* globalValues,
	const dsMatrix44f* viewMatrix, const dsFrustum3f* viewFrustum);

/**
 * @brief Destroys a particle drawer.
 * @remark errno will be set on failure.
 * @param drawer The particle drawer to destroy.
 * @return False if graphics resources couldn't be destroyed.
 */
DS_PARTICLE_EXPORT bool dsParticleDraw_destroy(dsParticleDraw* drawer);

#ifdef __cplusplus
}
#endif

