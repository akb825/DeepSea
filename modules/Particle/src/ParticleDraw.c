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

#include <DeepSea/Particle/ParticleDraw.h>

#include "ParticleEmitterInternal.h"

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/Lifetime.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Sort.h>

#include <DeepSea/Render/Resources/DrawGeometry.h>
#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/ResourceManager.h>
#include <DeepSea/Render/Renderer.h>

#define FRAME_DELAY 3

typedef struct BufferInfo
{
	dsGfxBuffer* buffer;
	dsDrawGeometry* geometry;
	uint64_t lastUsedFrame;
} BufferInfo;

typedef struct ParticleRef
{
	dsVector3f position;
	const dsParticle* particle;
} ParticleRef;

struct dsParticleDraw
{
	dsAllocator* allocator;

	dsLifetime* lifetime;

	dsResourceManager* resourceManager;
	dsAllocator* resourceAllocator;

	dsLifetime** emitters;
	uint32_t emitterCount;
	uint32_t maxEmitters;

	dsSpinlock emitterLock;

	ParticleRef* particles;
	uint32_t particleCount;
	uint32_t maxParticles;

	BufferInfo* buffers;
	uint32_t bufferCount;
	uint32_t maxBuffers;
};

dsParticleDraw* dsParticleDraw_create(dsAllocator* allocator, dsResourceManager* resourceManager,
	dsAllocator* resourceAllocator)
{
	if (!allocator || !resourceManager)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_PARTICLE_LOG_TAG,
			"Particle drawer allocator must support freeing memory.");
		return NULL;
	}

	if (!resourceAllocator)
		resourceAllocator = allocator;

	dsParticleDraw* drawer = DS_ALLOCATE_OBJECT(allocator, dsParticleDraw);
	if (!drawer)
		return NULL;

	drawer->lifetime = dsLifetime_create(allocator, drawer);
	if (!drawer->lifetime)
	{
		DS_VERIFY(dsAllocator_free(allocator, drawer));
		return NULL;
	}

	drawer->allocator = dsAllocator_keepPointer(allocator);
	drawer->resourceManager = resourceManager;
	drawer->resourceAllocator = resourceAllocator;

	drawer->emitters = NULL;
	drawer->emitterCount = 0;
	drawer->maxEmitters = 0;

	DS_VERIFY(dsSpinlock_initialize(&drawer->emitterLock));

	drawer->particles = NULL;
	drawer->particleCount = 0;
	drawer->maxParticles = 0;

	drawer->buffers = NULL;
	drawer->bufferCount = 0;
	drawer->maxBuffers = 0;

	return drawer;
}

bool dsParticleDraw_addEmitter(dsParticleDraw* drawer, dsParticleEmitter* emitter)
{
	if (!drawer || !emitter)
	{
		errno = EINVAL;
		return false;
	}

	DS_VERIFY(dsSpinlock_lock(&drawer->emitterLock));

	for (uint32_t i = 0; i < drawer->emitterCount; ++i)
	{
		if (drawer->emitters[i] == emitter->lifetime)
		{
			errno = EPERM;
			DS_VERIFY(dsSpinlock_unlock(&drawer->emitterLock));
			return false;
		}
	}

	uint32_t emitterIndex = drawer->emitterCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(drawer->allocator, drawer->emitters, drawer->emitterCount,
			drawer->maxEmitters, 1))
	{
		DS_VERIFY(dsSpinlock_unlock(&drawer->emitterLock));
		return false;
	}

	drawer->emitters[emitterIndex] = dsLifetime_addRef(emitter->lifetime);

	// Also add a reference to the emitter.
	if (!dsParticleEmitter_addDrawer(emitter, drawer->lifetime))
	{
		// Remove the emitter we just added if we couldn't add this to the emitter.
		--drawer->emitterCount;
		dsLifetime_freeRef(emitter->lifetime);
		DS_VERIFY(dsSpinlock_unlock(&drawer->emitterLock));
		return false;
	}

	DS_VERIFY(dsSpinlock_unlock(&drawer->emitterLock));
	return true;
}

bool dsParticleDraw_removeEmitter(dsParticleDraw* drawer, dsParticleEmitter* emitter)
{
	if (!drawer || !emitter)
	{
		errno = EINVAL;
		return false;
	}

	DS_VERIFY(dsSpinlock_lock(&drawer->emitterLock));

	uint32_t emitterIndex;
	for (emitterIndex = 0; emitterIndex < drawer->emitterCount; ++emitterIndex)
	{
		if (drawer->emitters[emitterIndex] == emitter->lifetime)
			break;
	}

	if (emitterIndex == drawer->emitterCount)
	{
		errno = ENOTFOUND;
		DS_VERIFY(dsSpinlock_unlock(&drawer->emitterLock));
		return false;
	}

	DS_VERIFY(DS_RESIZEABLE_ARRAY_REMOVE(drawer->emitters, drawer->emitterCount, emitterIndex, 1));

	// Also remove the reference from the emitter. Allow it to not exist. (e.g. on destruction)
	dsParticleEmitter_removeDrawer(emitter, drawer->lifetime);

	DS_VERIFY(dsSpinlock_unlock(&drawer->emitterLock));
	return true;
}

bool dsParticleDraw_destroy(dsParticleDraw* drawer)
{
	if (!drawer)
		return true;

	// First destroy the buffers to fail first if we can't destroy the resources.
	for (uint32_t i = 0; i < drawer->bufferCount; ++i)
	{
		if (!dsGfxBuffer_destroy(drawer->buffers[i].buffer))
		{
			DS_ASSERT(i == 0);
			return false;
		}
		DS_VERIFY(dsDrawGeometry_destroy(drawer->buffers[i].geometry));
	}
	DS_VERIFY(dsAllocator_free(drawer->allocator, drawer->buffers));

	// Clear out the array inside the lock to avoid nested locking that can result in deadlocks.
	DS_VERIFY(dsSpinlock_lock(&drawer->emitterLock));
	dsLifetime** emitters = drawer->emitters;
	uint32_t emitterCount = drawer->emitterCount;
	drawer->emitters = NULL;
	drawer->emitterCount = 0;
	drawer->maxEmitters = 0;
	DS_VERIFY(dsSpinlock_unlock(&drawer->emitterLock));

	// Remove this from all emitters.
	for (uint32_t i = 0; i < emitterCount; ++i)
	{
		dsParticleEmitter* emitter = (dsParticleEmitter*)dsLifetime_acquire(emitters[i]);
		if (emitter)
		{
			dsParticleEmitter_removeDrawer(emitter, drawer->lifetime);
			dsLifetime_release(emitters[i]);
		}
		dsLifetime_freeRef(emitters[i]);
	}
	DS_VERIFY(dsAllocator_free(drawer->allocator, emitters));
	dsLifetime_destroy(drawer->lifetime);
	dsSpinlock_shutdown(&drawer->emitterLock);

	DS_VERIFY(dsAllocator_free(drawer->allocator, drawer->particles));
	DS_VERIFY(dsAllocator_free(drawer->allocator, drawer));
	return true;
}
