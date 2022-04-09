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
	uint32_t seed;
	bool enabled;
	dsStandardParticleEmitterOptions options;
	float nextSpawnCountdown;
};

typedef struct dsStandardParticle
{
	dsParticle particle;
	float speed;
	float rotationSpeed;
	float timeScale;
} dsStandardParticle;

static uint32_t dsStandardParticleEmitter_update(dsParticleEmitter* emitter, double time,
	const uint8_t* curParticles, uint32_t curParticleCount, uint8_t* nextParticles)
{
	float timef = (float)time;
	// Update existing particles.
	uint32_t nextParticleCount = 0;
	const uint8_t* endCurParticles = curParticles + curParticleCount*emitter->sizeofParticle;
	uint8_t* nextParticlePtr = nextParticles;
	for (const uint8_t* curParticlePtr = curParticles; curParticlePtr < endCurParticles;
		curParticlePtr += emitter->sizeofParticle)
	{
		const dsParticle* curParticle = (const dsParticle*)curParticlePtr;
		const dsStandardParticle* curStandardParticle = (const dsStandardParticle*)curParticlePtr;
		float nextT = curParticle->t + (float)(curStandardParticle->timeScale*time);
		// Delete once the time has been exceeded.
		if (nextT > 1)
			continue;

		++nextParticleCount;
		dsParticle* nextParticle = (dsParticle*)nextParticlePtr;
		dsStandardParticle* nextStandardParticle = (dsStandardParticle*)nextParticlePtr;
		nextParticlePtr += emitter->sizeofParticle;

		dsVector3f offset;
		dsVector3_scale(offset, curParticle->direction, curStandardParticle->speed*timef);
		dsVector3_add(nextParticle->position, curParticle->position, offset);

		nextParticle->rotation.x = curParticle->rotation.x +
			(float)(curStandardParticle->rotationSpeed*time);
		nextParticle->rotation.x = dsWrapf(nextParticle->rotation.x, 0, (float)(2*M_PI));
		nextParticle->rotation.y = curParticle->rotation.y;

		nextParticle->color = curParticle->color;
		nextParticle->intensity = curParticle->intensity;
		nextParticle->textureIndex = curParticle->textureIndex;
		nextParticle->t = nextT;

		nextStandardParticle->speed = curStandardParticle->speed;
		nextStandardParticle->rotationSpeed = curStandardParticle->rotationSpeed;
		nextStandardParticle->timeScale = curStandardParticle->timeScale;
	}

	// Create any new particles.
	dsStandardParticleEmitter* standardEmitter = (dsStandardParticleEmitter*)emitter;
	standardEmitter->nextSpawnCountdown -= timef;
	if (standardEmitter->nextSpawnCountdown > 0 || nextParticleCount >= emitter->maxParticles)
		return nextParticleCount;

	const dsStandardParticleEmitterOptions* options = &standardEmitter->options;
	dsMatrix33f directionMatrix;
	dsParticle_createDirectionMatrix(&directionMatrix, &options->baseDirection);

	const dsVector2f rotationRange = {{0, (float)(2*M_PI)}};
	do
	{
		++nextParticleCount;
		dsParticle* nextParticle = (dsParticle*)nextParticlePtr;
		dsStandardParticle* nextStandardParticle = (dsStandardParticle*)nextParticlePtr;
		nextParticlePtr += emitter->sizeofParticle;

		dsParticle_randomPosition(nextParticle, &standardEmitter->seed, &options->spawnVolume,
			&options->volumeMatrix);
		dsParticle_randomDirection(nextParticle, &standardEmitter->seed, &directionMatrix,
			options->directionSpread);
		dsParticle_randomRotation(nextParticle, &standardEmitter->seed, &rotationRange,
			&rotationRange);
		dsParticle_randomColor(nextParticle, &standardEmitter->seed, &options->colorHueRange,
			&options->colorSaturationRange, &options->colorValueRange);
		dsParticle_randomIntensity(nextParticle, &standardEmitter->seed, &options->intensityRange);
		dsParticle_randomTexture(nextParticle, &standardEmitter->seed, &options->textureRange);
		nextParticle->t = 0;

		nextStandardParticle->speed = dsRandomFloat(&standardEmitter->seed, options->speedRange.x,
			options->speedRange.y);
		nextStandardParticle->rotationSpeed = dsRandomFloat(&standardEmitter->seed,
			options->rotationRange.x, options->rotationRange.y);
		nextStandardParticle->timeScale = 1/dsRandomFloat(&standardEmitter->seed,
			options->activeTimeRange.x, options->activeTimeRange.y);

		standardEmitter->nextSpawnCountdown += dsRandomFloat(&standardEmitter->seed,
			standardEmitter->options.spawnTimeRange.x,
			standardEmitter->options.spawnTimeRange.y);
	} while (standardEmitter->nextSpawnCountdown <= 0 &&
		nextParticleCount < emitter->maxParticles);
	return nextParticleCount;
}

static void dsStandardParticleEmitter_destroy(dsParticleEmitter* emitter)
{
	dsAllocator_free(emitter->allocator, emitter);
}

dsStandardParticleEmitter* dsStandardParticleEmitter_create(dsAllocator* allocator,
	uint32_t maxParticles, uint32_t seed, const dsStandardParticleEmitterOptions* options,
	bool enabled)
{
	if (!allocator || !options)
	{
		errno = EINVAL;
		return NULL;
	}

	dsStandardParticleEmitter* emitter = (dsStandardParticleEmitter*)dsParticleEmitter_create(
		allocator, sizeof(dsStandardParticleEmitter), sizeof(dsStandardParticle), maxParticles,
		&dsStandardParticleEmitter_update, &dsStandardParticleEmitter_destroy);
	if (!emitter)
		return NULL;

	emitter->seed = seed;
	emitter->enabled = enabled;
	emitter->options = *options;
	emitter->nextSpawnCountdown = 0;
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
