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
#include <DeepSea/Core/Types.h>
#include <DeepSea/Math/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Includes all of the types used in the DeepSea/Particle library.
 */

/**
 * @brief Log tag used by the particle library.
 */
#define DS_PARTICLE_LOG_TAG "particle"

/**
 * @brief Struct describing a single particle.
 *
 * Different particle emitters may allocate more space for each particle for extra state used by the
 * emitter.
 */
typedef struct dsParticle
{
	/**
	 * @brief The position of the particle.
	 */
	dsVector3f position;

	/**
	 * @brief The direction of the particle.
	 */
	dsVector3f direction;

	/**
	 * @brief The color of the particle.
	 */
	dsColor color;

	/**
	 * @brief The intensity of the particle.
	 *
	 * This can be used to make the color brighter, such as for emissive particles.
	 */
	float intensity;

	/**
	 * @brief Index of the texture when using a texture array.
	 */
	uint32_t textureIndex;

	/**
	 * @brief T value for the lifetime of the particle.
	 *
	 * This should be in the range [0, 1].
	 */
	float t;
} dsParticle;

/**
 * @brief Struct describing an emitter of particles.
 *
 * Different implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsParticleEmitter and the true internal type.
 *
 * @see ParticleEmitter.h
 */
typedef struct dsParticleEmitter dsParticleEmitter;

/**
 * @brief Function to update a particle emitter.
 * @param emitter The particle emitter to update.
 * @param time The time that has elapsed from the last update.
 * @param curParticles The current list of particles.
 * @param curParticleCount The number of currently active particles.
 * @param nextParticles The list of next particles to populate.
 * @param maxParticles The maximum number of particles that can be created.
 * @return The new number of particles.
 */
typedef uint32_t (*dsUpdateParticleEmitterFunction)(dsParticleEmitter* emitter, double time,
	dsParticle** curParticles, uint32_t curParticleCount, dsParticle** nextParticles,
	uint32_t maxParticles);

/**
 * @brief Function to destroy a particle emitter.
 * @param emitter The particle emitter to destroy.
 */
typedef void (*dsDestroyParticleEmitterFunction)(dsParticleEmitter* emitter);

/**
 * @copydoc dsParticleEmitter
 */
struct dsParticleEmitter
{
	/**
	 * @brief The allocator the particle emitter was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The list of active particles.
	 */
	dsParticle** particles;

	/**
	 * @brief Temporary list of particles used during processing.
	 */
	dsParticle** tempParticles;

	/**
	 * @brief The current number of particles.
	 */
	uint32_t particleCount;

	/**
	 * @brief The maximum number of particles that can be active at once.
	 */
	uint32_t maxParticles;

	/**
	 * @brief Pool of inactive particles.
	 */
	dsPoolAllocator inactiveParticles;

	/**
	 * @brief Function to update the particle emitter.
	 */
	dsUpdateParticleEmitterFunction updateFunc;

	/**
	 * @brief Function to destroy the particle emitter.
	 */
	dsDestroyParticleEmitterFunction destroyFunc;
};

#ifdef __cplusplus
}
#endif
