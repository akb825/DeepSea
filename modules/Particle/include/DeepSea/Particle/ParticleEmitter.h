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

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions to create and manipulate particle emitters.
 * @see dsParticleEmitter
 */

/**
 * @brief Creates a particle emitter.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the particle emitter from. This must support freeing
 *     memory.
 * @param sizeofParticleEmitter The size of the particle emitter structure.
 * @param sizeofParticle The size of the particle structure.
 * @param params The list of common particle emitter parameters.
 * @param updateFunc The function to update the particle emitter.
 * @param destroyFunc The function to destroy the particle emitter.
 * @return The particle emitter or NULL if an error occurred.
 */
DS_PARTICLE_EXPORT dsParticleEmitter* dsParticleEmitter_create(dsAllocator* allocator,
	size_t sizeofParticleEmitter, size_t sizeofParticle, const dsParticleEmitterParams* params,
	dsUpdateParticleEmitterFunction updateFunc, dsDestroyParticleEmitterFunction destroyFunc);

/**
 * @brief Updates a particle emitter.
 * @remark errno will be set on failure.
 * @param emitter The particle emitter to update.
 * @param time The time that's elapsed since the last update.
 * @return False if an error occurred.
 */
DS_PARTICLE_EXPORT bool dsParticleEmitter_update(dsParticleEmitter* emitter, float time);

/**
 * @brief Populates a set of instance values to use when drawing the particle emitter.
 * @remark errno will be set on failure.
 * @param emitter The particle emitter to populate the instance values for.
 * @param values The instance values to populate.
 * @return False if an error occurred.
 */
DS_PARTICLE_EXPORT bool dsparticleEmitter_populateInstanceValues(const dsParticleEmitter* emitter,
	dsSharedMaterialValues* values);

/**
 * @brief Destroys a particle emitter.
 * @param emitter The particle emitter to destroy.
 */
DS_PARTICLE_EXPORT void dsParticleEmitter_destroy(dsParticleEmitter* emitter);

#ifdef __cplusplus
}
#endif
