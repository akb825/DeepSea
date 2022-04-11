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
 * @brief Functions to create and manipulate standard particle emitters.
 * @see dsStandardParticleEmitter
 */

/**
 * @brief Creates a standard particle emitter.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the particle emitter with.
 * @param maxParticles The maximum number of particles that may be active.
 * @param seed The seed value for random values.
 * @param options The options for the particle emitter. This must not be NULL.
 * @param enabled Whether or not new particles will be created.
 * @param startTime The time to start the particle emitter at. The first frame this is updated the
 *     create particles and advance them to this time.
 * @return The particle emitter or NULL if an error occurred.
 */
DS_PARTICLE_EXPORT dsStandardParticleEmitter* dsStandardParticleEmitter_create(
	dsAllocator* allocator, uint32_t maxParticles, uint32_t seed,
	const dsStandardParticleEmitterOptions* options, bool enabled, float startTime);

/**
 * @brief Gets the options for the standard particle emitter.
 * @remark errno will be set on failure.
 * @param emitter The emitter to get the options for.
 * @return The emitter options or NULL if emitter is NULL.
 */
DS_PARTICLE_EXPORT const dsStandardParticleEmitterOptions* dsStandardParticleEmitter_getOptions(
	const dsStandardParticleEmitter* emitter);

/**
 * @brief Gets the mutable options for the standard particle emitter.
 * @remark errno will be set on failure.
 * @param emitter The emitter to get the options for.
 * @return The emitter options or NULL if emitter is NULL.
 */
DS_PARTICLE_EXPORT dsStandardParticleEmitterOptions* dsStandardParticleEmitter_getMutableOptions(
	dsStandardParticleEmitter* emitter);

/**
 * @brief Gets whether or not a standard particle emitter is enabled.
 *
 * When disabled, no new particles will be created.
 *
 * @param emitter The particle emitter.
 * @return Whether or not the particle emitter is enabled.
 */
DS_PARTICLE_EXPORT bool dsStandardParticleEmitter_getEnabled(
	const dsStandardParticleEmitter* emitter);

/**
 * @brief Sets whether or not a standard particle emitter is enabled.
 *
 * When disabled, no new particles will be created.
 *
 * @remark errno will be set on failure.
 * @param emitter The particle emitter.
 * @param enabled Whether or not the particle is enabled.
 * @return False if emitter is NULL.
 */
DS_PARTICLE_EXPORT bool dsStandardParticleEmitter_setEnabled(dsStandardParticleEmitter* emitter,
	bool enabled);

#ifdef __cplusplus
}
#endif
