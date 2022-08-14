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

#include <DeepSea/Particle/StandardParticleEmitter.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Random.h>
#include <DeepSea/Math/Vector3.h>

#include <DeepSea/Particle/ParticleEmitter.h>
#include <DeepSea/Particle/Particle.h>

struct dsStandardParticleEmitter
{
	dsParticleEmitter emitter;
	dsRandom random;
	bool enabled;
	dsStandardParticleEmitterOptions options;
	float nextSpawnCountdown;
};

typedef struct dsStandardParticle
{
	dsParticle particle;
	dsVector3f direction;
	float speed;
	float rotationSpeed;
	float timeScale;
} dsStandardParticle;

static bool advanceParticle(dsParticle* nextParticle, const dsParticle* prevParticle, float time)
{
	const dsStandardParticle* prevStandardParticle = (const dsStandardParticle*)prevParticle;
	float nextT = prevParticle->t + prevStandardParticle->timeScale*time;
	// Delete once the time has been exceeded.
	if (nextT > 1)
		return false;

	dsVector3f offset;
	dsVector3_scale(offset, prevStandardParticle->direction, prevStandardParticle->speed*time);
	dsVector3_add(nextParticle->position, prevParticle->position, offset);

	nextParticle->rotation.x = prevParticle->rotation.x + prevStandardParticle->rotationSpeed*time;
	nextParticle->rotation.x = dsWrapf(nextParticle->rotation.x, 0, (float)(2*M_PI));
	nextParticle->rotation.y = nextParticle->rotation.y;
	nextParticle->t = nextT;
	return true;
}

static uint32_t dsStandardParticleEmitter_update(dsParticleEmitter* emitter, float time,
	const uint8_t* curParticles, uint32_t curParticleCount, uint8_t* nextParticles)
{
	// Update existing particles.
	uint32_t nextParticleCount = 0;
	const uint8_t* endCurParticles = curParticles + curParticleCount*emitter->sizeofParticle;
	uint8_t* nextParticlePtr = nextParticles;
	for (const uint8_t* curParticlePtr = curParticles; curParticlePtr < endCurParticles;
		curParticlePtr += emitter->sizeofParticle)
	{
		const dsParticle* prevParticle = (const dsParticle*)curParticlePtr;
		dsParticle* nextParticle = (dsParticle*)nextParticlePtr;
		if (!advanceParticle(nextParticle, prevParticle, time))
			continue;

		const dsStandardParticle* prevStandardParticle = (const dsStandardParticle*)prevParticle;
		dsStandardParticle* nextStandardParticle = (dsStandardParticle*)nextParticle;

		// Copy over components that are static.
		nextParticle->color = prevParticle->color;
		nextParticle->intensity = prevParticle->intensity;
		nextParticle->textureIndex = prevParticle->textureIndex;

		nextStandardParticle->speed = prevStandardParticle->speed;
		nextStandardParticle->rotationSpeed = prevStandardParticle->rotationSpeed;
		nextStandardParticle->timeScale = prevStandardParticle->timeScale;

		++nextParticleCount;
		nextParticlePtr += emitter->sizeofParticle;
	}

	// Create any new particles based on the timer before adding a new particle and availability
	// based on the limit.
	dsStandardParticleEmitter* standardEmitter = (dsStandardParticleEmitter*)emitter;
	standardEmitter->nextSpawnCountdown -= time;
	if (standardEmitter->nextSpawnCountdown > 0 || nextParticleCount >= emitter->maxParticles)
		return nextParticleCount;

	const dsStandardParticleEmitterOptions* options = &standardEmitter->options;
	dsMatrix33f directionMatrix;
	dsParticle_createDirectionMatrix(&directionMatrix, &options->baseDirection);

	const dsVector2f rotationRange = {{0, (float)(2*M_PI)}};
	do
	{

		// Add the time before creating the next particle to the countdown timer.
		standardEmitter->nextSpawnCountdown += dsRandom_nextFloatRange(&standardEmitter->random,
			standardEmitter->options.spawnTimeRange.x,
			standardEmitter->options.spawnTimeRange.y);

		// Skip this point if not enabled. This avoids creating new particles, but still advances
		// the spawn counter so it can continue emitting when enabled.
		if (!standardEmitter->enabled)
			continue;

		// If time from the spawn cowntdown to 0 is the amount of time the newly created particle
		// has been alive for.
		float curElapsedTime = -standardEmitter->nextSpawnCountdown;

		float particleTime = dsRandom_nextFloatRange(&standardEmitter->random,
			options->activeTimeRange.x, options->activeTimeRange.y);
		// Skip this particle if it will expire with the remaining time.
		if (particleTime <= curElapsedTime)
			continue;

		++nextParticleCount;
		dsParticle* nextParticle = (dsParticle*)nextParticlePtr;
		dsStandardParticle* nextStandardParticle = (dsStandardParticle*)nextParticlePtr;
		nextParticlePtr += emitter->sizeofParticle;

		dsParticle_randomPosition(nextParticle, &standardEmitter->random, &options->spawnVolume,
			&options->spawnVolumeMatrix);
		dsParticle_randomSize(nextParticle, &standardEmitter->random, &options->widthRange,
			&options->heightRange);
		dsParticle_randomDirection(&nextStandardParticle->direction, &standardEmitter->random,
			&directionMatrix, options->directionSpread);
		dsParticle_randomRotation(nextParticle, &standardEmitter->random, &rotationRange,
			&rotationRange);
		dsParticle_randomColor(nextParticle, &standardEmitter->random, &options->colorHueRange,
			&options->colorSaturationRange, &options->colorValueRange);
		dsParticle_randomIntensity(nextParticle, &standardEmitter->random,
			&options->intensityRange);
		dsParticle_randomTexture(nextParticle, &standardEmitter->random, &options->textureRange);
		nextParticle->t = 0;

		nextStandardParticle->speed = dsRandom_nextFloatRange(&standardEmitter->random,
			options->speedRange.x, options->speedRange.y);
		nextStandardParticle->rotationSpeed = dsRandom_nextFloatRange(&standardEmitter->random,
			options->rotationSpeedRange.x, options->rotationSpeedRange.y);
		nextStandardParticle->timeScale = 1/particleTime;

		// Advance the particle based on the time it's been alive.
		advanceParticle(nextParticle, nextParticle, curElapsedTime);
	} while (standardEmitter->nextSpawnCountdown <= 0 &&
		nextParticleCount < emitter->maxParticles);
	return nextParticleCount;
}

static void dsStandardParticleEmitter_destroy(dsParticleEmitter* emitter)
{
	dsAllocator_free(emitter->allocator, emitter);
}

dsStandardParticleEmitter* dsStandardParticleEmitter_create(dsAllocator* allocator,
	const dsParticleEmitterParams* params, uint64_t seed,
	const dsStandardParticleEmitterOptions* options, bool enabled, float startTime)
{
	if (!allocator || !params || !options)
	{
		errno = EINVAL;
		return NULL;
	}

	dsStandardParticleEmitter* emitter = (dsStandardParticleEmitter*)dsParticleEmitter_create(
		allocator, sizeof(dsStandardParticleEmitter), sizeof(dsStandardParticle), params,
		&dsStandardParticleEmitter_update, &dsStandardParticleEmitter_destroy);
	if (!emitter)
		return NULL;

	dsRandom_seed(&emitter->random, seed);
	emitter->enabled = enabled;
	emitter->options = *options;
	emitter->nextSpawnCountdown = -startTime;
	return emitter;
}

const dsStandardParticleEmitterOptions* dsStandardParticleEmitter_getOptions(
	const dsStandardParticleEmitter* emitter)
{
	if (!emitter)
	{
		errno = EINVAL;
		return NULL;
	}

	return &emitter->options;
}

dsStandardParticleEmitterOptions* dsStandardParticleEmitter_getMutableOptions(
	dsStandardParticleEmitter* emitter)
{
	if (!emitter)
	{
		errno = EINVAL;
		return NULL;
	}

	return &emitter->options;
}

bool dsStandardParticleEmitter_getEnabled(const dsStandardParticleEmitter* emitter)
{
	return emitter && emitter->enabled;
}

bool dsStandardParticleEmitter_setEnabled(dsStandardParticleEmitter* emitter, bool enabled)
{
	if (!emitter)
	{
		errno = EINVAL;
		return false;
	}

	emitter->enabled = enabled;
	return true;
}
