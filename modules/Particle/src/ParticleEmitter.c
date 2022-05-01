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

#include "ParticleEmitterInternal.h"

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/Lifetime.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Geometry/AlignedBox3.h>
#include <DeepSea/Geometry/OrientedBox3.h>

#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Vector3.h>

#include <DeepSea/Particle/ParticleDraw.h>

#include <DeepSea/Render/Resources/Material.h>

dsParticleEmitter* dsParticleEmitter_create(dsAllocator* allocator, size_t sizeofParticleEmitter,
	size_t sizeofParticle, const dsParticleEmitterParams* params,
	dsUpdateParticleEmitterFunction updateFunc, dsDestroyParticleEmitterFunction destroyFunc)
{
	if (!allocator || sizeofParticleEmitter < sizeof(dsParticleEmitter) ||
		sizeofParticle < sizeof(dsParticle) || !params || params->maxParticles == 0 ||
		!params->shader || !params->material || !updateFunc || !destroyFunc)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_PARTICLE_LOG_TAG,
			"Particle emitter allocator must support freeing memory.");
		return NULL;
	}

	uint32_t materialInstanceValueCount = 0;
	const dsMaterialDesc* materialDesc = dsMaterial_getDescription(params->material);
	for (uint32_t i = 0; i < materialDesc->elementCount; ++i)
	{
		materialInstanceValueCount +=
			materialDesc->elements[i].binding == dsMaterialBinding_Instance;
	}
	uint32_t instanceValueCount = dsMax(params->instanceValueCount, materialInstanceValueCount);
	if (instanceValueCount > 0 && !params->populateInstanceValuesFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_PARTICLE_LOG_TAG, "Particle must have a populate instance values function "
			"if instance values are present.");
		return NULL;
	}

	size_t fullSize = DS_ALIGNED_SIZE(sizeofParticleEmitter) +
		DS_ALIGNED_SIZE(sizeofParticle*params->maxParticles)*2;
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
		sizeofParticle*params->maxParticles);
	DS_ASSERT(emitter->particles);
	emitter->tempParticles = (uint8_t*)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
		sizeofParticle*params->maxParticles);
	DS_ASSERT(emitter->tempParticles);
	emitter->sizeofParticle = (uint32_t)sizeofParticle;
	emitter->particleCount = 0;
	emitter->maxParticles = params->maxParticles;

	emitter->shader = params->shader;
	emitter->material = params->material;
	emitter->instanceValueCount = instanceValueCount;

	dsMatrix44_identity(emitter->transform);
	dsOrientedBox3_makeInvalid(emitter->bounds);

	emitter->updateFunc = updateFunc;
	emitter->populateInstanceValuesFunc = params->populateInstanceValuesFunc;
	emitter->populateInstanceValuesUserData = params->populateInstanceValuesUserData;
	emitter->destroyFunc = destroyFunc;

	emitter->drawers = NULL;
	emitter->drawerCount = 0;
	emitter->maxDrawers = 0;

	emitter->lifetime = dsLifetime_create(allocator, emitter);
	if (!emitter->lifetime)
	{
		DS_VERIFY(dsAllocator_free(allocator, emitter));
		return NULL;
	}

	DS_VERIFY(dsSpinlock_initialize(&emitter->drawerLock));
	return emitter;
}

bool dsParticleEmitter_update(dsParticleEmitter* emitter, float time)
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

	// Update the bounds once we've gotten the full particle list.
	dsAlignedBox3f baseBounds;
	dsAlignedBox3f_makeInvalid(&baseBounds);

	uint8_t* particleEnd = emitter->particles + emitter->particleCount*emitter->sizeofParticle;
	for (uint8_t* particlePtr = emitter->particles; particlePtr < particleEnd;
		particlePtr += emitter->sizeofParticle)
	{
		dsParticle* particle = (dsParticle*)particlePtr;

		// Take the maximum volume the particle can occupy.
		float maxOffset = (float)(M_SQRT2*dsMax(particle->size.x, particle->size.y));
		dsVector3f offset = {{maxOffset, maxOffset, maxOffset}};

		dsVector3f position;
		dsVector3_add(position, particle->position, offset);
		dsAlignedBox3_addPoint(baseBounds, position);

		dsVector3_sub(position, particle->position, offset);
		dsAlignedBox3_addPoint(baseBounds, position);
	}

	if (dsAlignedBox3_isValid(baseBounds))
	{
		dsOrientedBox3f_fromAlignedBox(&emitter->bounds, &baseBounds);
		dsOrientedBox3f_transform(&emitter->bounds, &emitter->transform);
	}
	else
		dsOrientedBox3_makeInvalid(emitter->bounds);

	return true;
}

bool dsParticleEmitter_populateInstanceValues(const dsParticleEmitter* emitter,
	dsSharedMaterialValues* values)
{
	if (!emitter || !values)
	{
		errno = EINVAL;
		return false;
	}

	if (emitter->populateInstanceValuesFunc)
	{
		emitter->populateInstanceValuesFunc(emitter, emitter->populateInstanceValuesUserData,
			values);
	}

	return true;
}

void dsParticleEmitter_destroy(dsParticleEmitter* emitter)
{
	if (!emitter)
		return;

	// Clear out the array inside the lock to avoid nested locking that can result in deadlocks.
	DS_VERIFY(dsSpinlock_lock(&emitter->drawerLock));
	dsLifetime** drawers = emitter->drawers;
	uint32_t drawerCount = emitter->drawerCount;
	emitter->drawers = NULL;
	emitter->drawerCount = 0;
	emitter->maxDrawers = 0;
	DS_VERIFY(dsSpinlock_unlock(&emitter->drawerLock));

	for (uint32_t i = 0; i < drawerCount; ++i)
	{
		dsParticleDraw* drawer = (dsParticleDraw*)dsLifetime_acquire(drawers[i]);
		if (drawer)
		{
			dsParticleDraw_removeEmitter(drawer, emitter);
			dsLifetime_release(drawers[i]);
		}
		dsLifetime_freeRef(drawers[i]);
	}

	dsLifetime_destroy(emitter->lifetime);
	dsSpinlock_shutdown(&emitter->drawerLock);

	if (emitter->destroyFunc)
		emitter->destroyFunc(emitter);
}

bool dsParticleEmitter_addDrawer(dsParticleEmitter* emitter, dsLifetime* drawer)
{
	DS_ASSERT(emitter);
	DS_ASSERT(drawer);

	DS_VERIFY(dsSpinlock_lock(&emitter->drawerLock));

	uint32_t drawerIndex = emitter->drawerCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(emitter->allocator, emitter->drawers, emitter->drawerCount,
			emitter->maxDrawers, 1))
	{
		DS_VERIFY(dsSpinlock_unlock(&emitter->drawerLock));
		return false;
	}

	emitter->drawers[drawerIndex] = dsLifetime_addRef(drawer);

	DS_VERIFY(dsSpinlock_unlock(&emitter->drawerLock));
	return true;
}

bool dsParticleEmitter_removeDrawer(dsParticleEmitter* emitter, dsLifetime* drawer)
{
	DS_VERIFY(dsSpinlock_lock(&emitter->drawerLock));

	uint32_t drawerIndex;
	for (drawerIndex = 0; drawerIndex < emitter->drawerCount; ++drawerIndex)
	{
		if (emitter->drawers[drawerIndex] == drawer)
			break;
	}

	bool exists = drawerIndex < emitter->drawerCount;
	if (exists)
	{
		DS_VERIFY(DS_RESIZEABLE_ARRAY_REMOVE(
			emitter->drawers, emitter->drawerCount, drawerIndex, 1));
		dsLifetime_freeRef(drawer);
	}

	DS_VERIFY(dsSpinlock_unlock(&emitter->drawerLock));
	return exists;
}
