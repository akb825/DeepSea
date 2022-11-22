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
 * @brief Gets the type of a standard particle emitter.
 * @return The type of a standard particle emitter.
 */
DS_PARTICLE_EXPORT dsParticleEmitterType dsStandardParticleEmitter_type(void);

/**
 * @brief Creates a standard particle emitter.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the particle emitter with.
 * @param params The list of common particle emitter parameters.
 * @param seed The seed value for random values.
 * @param options The options for the particle emitter. This must not be NULL.
 * @param startTime The time to start the particle emitter at. The first frame this is updated the
 *     create particles and advance them to this time.
 * @return The particle emitter or NULL if an error occurred.
 */
DS_PARTICLE_EXPORT dsStandardParticleEmitter* dsStandardParticleEmitter_create(
	dsAllocator* allocator, const dsParticleEmitterParams* params, uint64_t seed,
	const dsStandardParticleEmitterOptions* options, float startTime);

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

#ifdef __cplusplus
}
#endif
