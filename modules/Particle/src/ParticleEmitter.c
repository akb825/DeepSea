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

#include <DeepSea/Particle/ParticleEmitter.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

dsParticleEmitter* dsParticleEmitter_create(dsAllocator* allocator, size_t sizeofParticleEmitter,
	size_t sizeofParticle, uint32_t maxParticles, dsUpdateParticleEmitterFunction updateFunc,
	dsDestroyParticleEmitterFunction destroyFunc)
{
	if (!allocator || sizeofParticleEmitter < sizeof(dsParticleEmitter) ||
		sizeofParticle < sizeof(dsParticle) || maxParticles == 0 || !updateFunc)
	{
		errno = EINVAL;
		return NULL;
	}

	size_t fullSize = DS_ALIGNED_SIZE(sizeofParticleEmitter) +
		DS_ALIGNED_SIZE(sizeofParticle*maxParticles)*2;
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsParticleEmitter* emitter =
		(dsParticleEmitter*)dsAllocator_alloc((dsAllocator*)&bufferAlloc, sizeofParticleEmitter);
	DS_ASSERT(emitter);

	emitter->allocator = dsAllocator_keepPointer(allocator);
	emitter->particles = (uint8_t*)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
		sizeofParticle*maxParticles);
	DS_ASSERT(emitter->particles);
	emitter->tempParticles = (uint8_t*)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
		sizeofParticle*maxParticles);
	DS_ASSERT(emitter->tempParticles);
	emitter->sizeofParticle = (uint32_t)sizeofParticle;
	emitter->particleCount = 0;
	emitter->maxParticles = maxParticles;

	emitter->updateFunc = updateFunc;
	emitter->destroyFunc = destroyFunc;
	return emitter;
}

bool dsParticleEmitter_update(dsParticleEmitter* emitter, double time)
{
	if (!emitter || time < 0)
	{
		errno = EINVAL;
		return false;
	}

	uint8_t* curParticles = emitter->particles;
	uint8_t* nextParticles = emitter->tempParticles;
	uint32_t nextParticleCount = emitter->updateFunc(emitter, time, curParticles,
		emitter->particleCount, nextParticles);
	// Assert since there's no way to cleanly recover, and this would be invalid interface usage.
	DS_ASSERT(nextParticleCount <= emitter->maxParticles);

	emitter->particles = nextParticles;
	emitter->tempParticles = curParticles;
	emitter->particleCount = nextParticleCount;
	return true;
}

void dsParticleEmitter_destroy(dsParticleEmitter* emitter)
{
	if (!emitter || !emitter->destroyFunc)
		return;

	emitter->destroyFunc(emitter);
}
