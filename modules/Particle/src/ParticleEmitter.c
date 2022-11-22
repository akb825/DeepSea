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
#include <DeepSea/Core/Log.h>

#include <DeepSea/Geometry/AlignedBox3.h>
#include <DeepSea/Geometry/OrientedBox3.h>

#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Vector3.h>

#include <DeepSea/Render/Resources/Material.h>

dsParticleEmitter* dsParticleEmitter_create(dsAllocator* allocator, dsParticleEmitterType type,
	size_t sizeofParticleEmitter, size_t sizeofParticle, const dsParticleEmitterParams* params,
	dsUpdateParticleEmitterFunction updateFunc, dsDestroyParticleEmitterFunction destroyFunc)
{
	if (!allocator || !type || sizeofParticleEmitter < sizeof(dsParticleEmitter) ||
		sizeofParticle < sizeof(dsParticle) || !params || params->maxParticles == 0 ||
		!params->shader || !params->material || !updateFunc || !destroyFunc)
	{
		errno = EINVAL;
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
	emitter->type = type;
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
	emitter->enabled = params->enabled;
	dsOrientedBox3_makeInvalid(emitter->bounds);

	emitter->updateFunc = updateFunc;
	emitter->populateInstanceValuesFunc = params->populateInstanceValuesFunc;
	emitter->populateInstanceValuesUserData = params->populateInstanceValuesUserData;
	emitter->destroyFunc = destroyFunc;

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
	dsSharedMaterialValues* values, uint32_t index, void* drawData)
{
	if (!emitter || !values)
	{
		errno = EINVAL;
		return false;
	}

	if (emitter->populateInstanceValuesFunc)
	{
		return emitter->populateInstanceValuesFunc(emitter, emitter->populateInstanceValuesUserData,
			values, index, drawData);
	}

	return true;
}

void dsParticleEmitter_destroy(dsParticleEmitter* emitter)
{
	if (!emitter || !emitter->destroyFunc)
		return;

	emitter->destroyFunc(emitter);
}
