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

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>

#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Packing.h>

#include <DeepSea/Particle/ParticleEmitter.h>

#include <DeepSea/Render/Resources/DrawGeometry.h>
#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/ResourceManager.h>
#include <DeepSea/Render/Resources/Shader.h>
#include <DeepSea/Render/Resources/SharedMaterialValues.h>
#include <DeepSea/Render/Resources/VertexFormat.h>
#include <DeepSea/Render/Renderer.h>

#include <limits.h>
#include <stdlib.h>

#define FRAME_DELAY 3
#define MAX_INDEX (USHRT_MAX - 1)
#define VERTEX_COUNT 4
#define INDEX_COUNT 6

typedef struct BufferInfo
{
	size_t maxParticles;
	dsGfxBuffer* buffer;
	dsDrawGeometry* geometry;
	uint64_t lastUsedFrame;
} BufferInfo;

typedef struct ParticleRef
{
	float viewZ;
	uint32_t emitter;
	const dsParticle* particle;
} ParticleRef;

typedef struct ParticleVertex
{
	dsVector3f position;
	dsHalfFloat offset[2];
	dsHalfFloat rotation[2];
	dsColor color;
	dsHalfFloat intensityTextureT[4];
} ParticleVertex;

struct dsParticleDraw
{
	dsAllocator* allocator;

	dsResourceManager* resourceManager;
	dsAllocator* resourceAllocator;

	dsSharedMaterialValues* instanceValues;

	ParticleRef* particles;
	uint32_t maxParticles;

	BufferInfo* buffers;
	uint32_t bufferCount;
	uint32_t maxBuffers;
};

static BufferInfo* getDrawBuffer(dsParticleDraw* draw, uint32_t particleCount,
	uint32_t maxParticles)
{
	uint64_t frameNumber = draw->resourceManager->renderer->frameNumber;
	// Look for any buffer with space for at least particleCount particles, but allocate based on
	// maxParticles to ensure greater stability of allocations.
	BufferInfo* bufferInfo = NULL;
	for (uint32_t i = 0; i < draw->bufferCount;)
	{
		BufferInfo* curBufferInfo = draw->buffers + i;

		// Skip over all buffers that are still in use, even if a different size.
		if (curBufferInfo->lastUsedFrame + FRAME_DELAY > frameNumber)
		{
			++i;
			continue;
		}

		if (curBufferInfo->maxParticles >= particleCount)
		{
			// Found. Only take the first one, and continue so that invalid buffers can be removed.
			if (!curBufferInfo)
			{
				curBufferInfo->lastUsedFrame = frameNumber;
				bufferInfo = curBufferInfo;
			}
			++i;
			continue;
		}

		// This buffer is too small. Delete it now since a new one will need to be allocated.
		if (!dsGfxBuffer_destroy(curBufferInfo->buffer))
			return false;

		// Constant-time removal since order doesn't matter.
		*curBufferInfo = draw->buffers[draw->bufferCount - 1];
		--draw->bufferCount;
	}

	if (bufferInfo)
		return bufferInfo;

	// Not found: create a new buffer.
	uint32_t vertexCount = maxParticles*VERTEX_COUNT;
	uint32_t indexCount = maxParticles*INDEX_COUNT;
	size_t vertexSize = vertexCount*sizeof(ParticleVertex);
	size_t indexSize = indexCount*sizeof(uint16_t);
	size_t bufferSize = vertexSize + indexSize;

	uint32_t index = draw->bufferCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(draw->allocator, draw->buffers, draw->bufferCount,
			draw->maxBuffers, 1))
	{
		return NULL;
	}

	bufferInfo = draw->buffers + index;
	bufferInfo->maxParticles = maxParticles;
	bufferInfo->lastUsedFrame = frameNumber;

	bufferInfo->buffer = dsGfxBuffer_create(draw->resourceManager, draw->resourceAllocator,
		dsGfxBufferUsage_Vertex | dsGfxBufferUsage_Index,
		dsGfxMemory_Draw | dsGfxMemory_Stream | dsGfxMemory_Synchronize, NULL, bufferSize);
	if (!bufferInfo->buffer)
	{
		--draw->bufferCount;
		return NULL;
	}

	dsVertexBuffer vertexBuffer =
	{
		bufferInfo->buffer,
		0,
		vertexCount
	};

	DS_VERIFY(dsVertexFormat_initialize(&vertexBuffer.format));
	vertexBuffer.format.elements[dsVertexAttrib_Position0].format =
		dsGfxFormat_decorate(dsGfxFormat_X32Y32Z32, dsGfxFormat_Float);
	vertexBuffer.format.elements[dsVertexAttrib_Position1].format =
		dsGfxFormat_decorate(dsGfxFormat_X16Y16, dsGfxFormat_Float);
	vertexBuffer.format.elements[dsVertexAttrib_Normal].format =
		dsGfxFormat_decorate(dsGfxFormat_X16Y16, dsGfxFormat_Float);
	vertexBuffer.format.elements[dsVertexAttrib_Color].format =
		dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
	vertexBuffer.format.elements[dsVertexAttrib_TexCoord0].format =
		dsGfxFormat_decorate(dsGfxFormat_X16Y16Z16W16, dsGfxFormat_Float);
	DS_VERIFY(dsVertexFormat_setAttribEnabled(
		&vertexBuffer.format, dsVertexAttrib_Position0, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(
		&vertexBuffer.format, dsVertexAttrib_Position1, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(&vertexBuffer.format, dsVertexAttrib_Normal, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(&vertexBuffer.format, dsVertexAttrib_Color, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(
		&vertexBuffer.format, dsVertexAttrib_TexCoord0, true));

	DS_VERIFY(vertexBuffer.format.size == sizeof(ParticleVertex));
	DS_ASSERT(vertexBuffer.format.elements[dsVertexAttrib_Position0].offset ==
		offsetof(ParticleVertex, position));
	DS_ASSERT(vertexBuffer.format.elements[dsVertexAttrib_Position1].offset ==
		offsetof(ParticleVertex, offset));
	DS_ASSERT(vertexBuffer.format.elements[dsVertexAttrib_Normal].offset ==
		offsetof(ParticleVertex, rotation));
	DS_ASSERT(vertexBuffer.format.elements[dsVertexAttrib_Color].offset ==
		offsetof(ParticleVertex, color));
	DS_ASSERT(vertexBuffer.format.elements[dsVertexAttrib_TexCoord0].offset ==
		offsetof(ParticleVertex, intensityTextureT));

	dsVertexBuffer* vertexBuffers[DS_MAX_GEOMETRY_VERTEX_BUFFERS] =
		{&vertexBuffer, NULL, NULL, NULL};

	dsIndexBuffer indexBuffer =
	{
		bufferInfo->buffer,
		vertexSize,
		indexCount,
		(uint32_t)sizeof(uint16_t)
	};

	bufferInfo->geometry = dsDrawGeometry_create(draw->resourceManager, draw->resourceAllocator,
		vertexBuffers, &indexBuffer);
	if (!bufferInfo->geometry)
	{
		DS_VERIFY(dsGfxBuffer_destroy(bufferInfo->buffer));
		--draw->bufferCount;
		return NULL;
	}

	return bufferInfo;
}

static int particleRefCompare(const void* left, const void* right)
{
	// Sort from for to near, so invert of comparing view Z positions.
	const ParticleRef* leftRef = (ParticleRef*)left;
	const ParticleRef* rightRef = (ParticleRef*)right;
	if (leftRef->viewZ > rightRef->viewZ)
		return -1;
	else if (leftRef->viewZ < rightRef->viewZ)
		return 1;
	return 0;
}

static void collectParticles(dsParticleDraw* drawer, const dsMatrix44f* viewMatrix,
	const dsParticleEmitter* const* emitters, uint32_t emitterCount, uint32_t particleCount)
{
	DS_PROFILE_FUNC_START();

	uint32_t curParticleCount = 0;
	ParticleRef* curParticleRef = drawer->particles;
	for (uint32_t i = 0; i < emitterCount; ++i)
	{
		const dsParticleEmitter* emitter = emitters[i];

		dsMatrix44f worldView;
		dsMatrix44_mul(worldView, *viewMatrix, emitter->transform);

		curParticleCount += emitter->particleCount;
		DS_ASSERT(curParticleCount <= drawer->maxParticles);
		DS_ASSERT(curParticleCount <= particleCount);
		const uint8_t* particlePtrEnd =
			emitter->particles + emitter->particleCount*emitter->sizeofParticle;
		for (const uint8_t* particlePtr = emitter->particles; particlePtr < particlePtrEnd;
			particlePtr += emitter->sizeofParticle, ++curParticleRef)
		{
			DS_ASSERT(curParticleRef < drawer->particles + curParticleCount);
			const dsParticle* particle = (const dsParticle*)particlePtr;
			// Only care about view Z coordinate, so save doing a full matrix transform.
			curParticleRef->viewZ = worldView.values[0][2]*particle->position.x +
				worldView.values[1][2]*particle->position.y +
				worldView.values[2][2]*particle->position.z + worldView.values[3][2];
			curParticleRef->emitter = i;
			curParticleRef->particle = particle;
		}
	}

	DS_UNUSED(curParticleCount);
	DS_ASSERT(curParticleCount == particleCount);
	DS_ASSERT(curParticleRef == drawer->particles + particleCount);
	qsort(drawer->particles, particleCount, sizeof(ParticleRef), &particleRefCompare);

	DS_PROFILE_FUNC_RETURN_VOID();
}

static bool populateParticleGeometry(dsParticleDraw* drawer, BufferInfo* bufferInfo,
	uint32_t particleCount)
{
	DS_PROFILE_FUNC_START();

	void* bufferData = dsGfxBuffer_map(bufferInfo->buffer, dsGfxBufferMap_Write, 0,
		DS_MAP_FULL_BUFFER);
	if (!bufferData)
		DS_PROFILE_FUNC_RETURN(false);

	ParticleVertex* vertices = (ParticleVertex*)bufferData;
	uint16_t* indices =
		(uint16_t*)(((uint8_t*)bufferData) + bufferInfo->geometry->indexBuffer.offset);

	uint32_t curIndex = 0;
	uint32_t prevEmitter = 0;
	for (uint32_t i = 0; i < particleCount; ++i, indices += INDEX_COUNT, curIndex += VERTEX_COUNT)
	{
		DS_ASSERT(vertices + VERTEX_COUNT <=
			(ParticleVertex*)bufferData + bufferInfo->geometry->vertexBuffers[0].count);
		DS_ASSERT(indices + INDEX_COUNT <=
			(uint16_t*)((uint8_t*)bufferData + bufferInfo->geometry->indexBuffer.offset) +
				bufferInfo->geometry->indexBuffer.count);

		const ParticleRef* particleRef = drawer->particles + i;
		const dsParticle* particle = particleRef->particle;
		dsHalfFloat packedOffsets[4][2] =
		{
			{dsPackHalfFloat(-particle->size.x/2), dsPackHalfFloat(-particle->size.y/2)},
			{dsPackHalfFloat(particle->size.x/2), dsPackHalfFloat(-particle->size.y/2)},
			{dsPackHalfFloat(particle->size.x/2), dsPackHalfFloat(particle->size.y/2)},
			{dsPackHalfFloat(-particle->size.x/2), dsPackHalfFloat(particle->size.y/2)}
		};
		dsHalfFloat packedRotation[2] = {dsPackHalfFloat(particle->rotation.x),
			dsPackHalfFloat(particle->rotation.y)};
		dsHalfFloat packedIntensityTextureT[4] =  {dsPackHalfFloat(particle->intensity),
			dsPackHalfFloat((float)particle->textureIndex), dsPackHalfFloat(particle->t), {0}};
		for (unsigned int j = 0; j < 4; ++j, ++vertices)
		{
			vertices->position = particle->position;
			vertices->offset[0] = packedOffsets[i][0];
			vertices->offset[1] = packedOffsets[i][1];
			vertices->rotation[0] = packedRotation[0];
			vertices->rotation[1] = packedRotation[1];
			vertices->color = particle->color;
			vertices->intensityTextureT[0] = packedIntensityTextureT[0];
			vertices->intensityTextureT[1] = packedIntensityTextureT[1];
			vertices->intensityTextureT[2] = packedIntensityTextureT[2];
			vertices->intensityTextureT[3] = packedIntensityTextureT[3];
		}

		// Need to reset the index offset if we switch emitters or exceed the maximum index.
		if (particleRef->emitter != prevEmitter || curIndex + VERTEX_COUNT > MAX_INDEX)
		{
			curIndex = 0;
			prevEmitter = particleRef->emitter;
		}

		indices[0] = (uint16_t)curIndex;
		indices[1] = (uint16_t)(curIndex + 1);
		indices[2] = (uint16_t)(curIndex + 2);

		indices[3] = (uint16_t)curIndex;
		indices[4] = (uint16_t)(curIndex + 2);
		indices[5] = (uint16_t)(curIndex + 3);
	}
	DS_ASSERT(vertices ==
		(ParticleVertex*)bufferData + bufferInfo->geometry->vertexBuffers[0].count);
	DS_ASSERT(indices ==
		(uint16_t*)((uint8_t*)bufferData + bufferInfo->geometry->indexBuffer.offset) +
			bufferInfo->geometry->indexBuffer.count);

	DS_VERIFY(dsGfxBuffer_unmap(bufferInfo->buffer));
	DS_PROFILE_FUNC_RETURN(true);
}

static bool drawParticles(dsParticleDraw* drawer, const dsParticleEmitter* const* emitters,
	uint32_t emitterCount, BufferInfo* bufferInfo, uint32_t particleCount,
	dsCommandBuffer* commandBuffer, const dsSharedMaterialValues* globalValues, void* drawData)
{
	DS_PROFILE_FUNC_START();

	uint32_t indexCount = 0;
	uint32_t startParticle = 0;
	uint32_t prevEmitter = (uint32_t)-1;
	dsShader* prevShader = NULL;
	dsMaterial* prevMaterial = NULL;
	for (uint32_t i = 0; i < particleCount; ++i, indexCount += INDEX_COUNT)
	{
		const ParticleRef* particleRef = drawer->particles + i;
		// Draw on emitter change or index overflow.
		bool changeEmitter = prevEmitter != particleRef->emitter;
		if (changeEmitter || indexCount + INDEX_COUNT > MAX_INDEX)
		{
			DS_ASSERT(prevShader);
			dsDrawIndexedRange drawRange =
			{
				indexCount,
				1,
				startParticle*INDEX_COUNT,
				startParticle*VERTEX_COUNT,
				0
			};

			if (!dsRenderer_drawIndexed(commandBuffer->renderer, commandBuffer, bufferInfo->geometry,
					&drawRange, dsPrimitiveType_TriangleList))
			{
				DS_VERIFY(dsShader_unbind(prevShader, commandBuffer));
				DS_PROFILE_FUNC_RETURN(false);
			}

			startParticle = 0;
			particleCount = 0;
		}

		if (changeEmitter)
		{
			// Prepare for the next batch of particles when the emitters changes
			prevEmitter = particleRef->emitter;
			const dsParticleEmitter* emitter = emitters[particleRef->emitter];
			DS_ASSERT(emitter);
			if (drawer->instanceValues)
			{
				DS_VERIFY(dsSharedMaterialValues_clear(drawer->instanceValues));
				if (!dsParticleEmitter_populateInstanceValues(emitter, drawer->instanceValues,
						particleRef->emitter, drawData))
				{
					if (prevShader)
						DS_VERIFY(dsShader_unbind(prevShader, commandBuffer));
					DS_PROFILE_FUNC_RETURN(false);
				}
			}

			if (emitter->shader != prevShader || emitter->material != prevMaterial)
			{
				if (prevShader)
					DS_VERIFY(dsShader_unbind(prevShader, commandBuffer));
				if (!dsShader_bind(emitter->shader, commandBuffer, emitter->material,
						globalValues, NULL))
				{
					DS_PROFILE_FUNC_RETURN(false);
				}

				prevShader = emitter->shader;
				prevMaterial = emitter->material;
			}

			if (drawer->instanceValues)
			{
				if (!dsShader_updateInstanceValues(prevShader, commandBuffer,
						drawer->instanceValues))
				{
					DS_VERIFY(dsShader_unbind(prevShader, commandBuffer));
					DS_PROFILE_FUNC_RETURN(false);
				}
			}
		}
	}

	// Draw any remaining particles.
	DS_ASSERT((prevShader && indexCount > 0) || (!prevShader && indexCount == 0));
	if (indexCount > 0)
	{
		dsDrawIndexedRange drawRange =
		{
			indexCount,
			1,
			startParticle*INDEX_COUNT,
			startParticle*VERTEX_COUNT,
			0
		};

		bool success = dsRenderer_drawIndexed(commandBuffer->renderer, commandBuffer,
			bufferInfo->geometry, 	&drawRange, dsPrimitiveType_TriangleList);

		DS_VERIFY(dsShader_unbind(prevShader, commandBuffer));
		DS_PROFILE_FUNC_RETURN(success);
	}

	DS_PROFILE_FUNC_RETURN(true);
}

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

	drawer->allocator = dsAllocator_keepPointer(allocator);
	drawer->resourceManager = resourceManager;
	drawer->resourceAllocator = resourceAllocator;

	drawer->instanceValues = NULL;

	drawer->particles = NULL;
	drawer->maxParticles = 0;

	drawer->buffers = NULL;
	drawer->bufferCount = 0;
	drawer->maxBuffers = 0;

	return drawer;
}

bool dsParticleDraw_draw(dsParticleDraw* drawer, dsCommandBuffer* commandBuffer,
	const dsSharedMaterialValues* globalValues, const dsMatrix44f* viewMatrix,
	const dsParticleEmitter* const* emitters, uint32_t emitterCount, void* drawData)
{
	DS_PROFILE_FUNC_START();

	if (!drawer || !commandBuffer || !globalValues || !viewMatrix ||
		(!emitters && emitterCount > 0))
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	uint32_t maxInstanceValues = 0;
	uint32_t maxParticles = 0;
	uint32_t particleCount = 0;
	for (uint32_t i = 0; i < emitterCount; ++i)
	{
		const dsParticleEmitter* emitter = emitters[i];
		if (!emitter)
		{
			errno = EINVAL;
			DS_PROFILE_FUNC_RETURN(false);
		}

		maxInstanceValues = dsMax(maxInstanceValues, emitter->instanceValueCount);
		maxParticles += emitter->maxParticles;
		particleCount += emitter->particleCount;
	}

	if (particleCount == 0)
		DS_PROFILE_FUNC_RETURN(false);

	// Make sure that instance values is large enough to hold the maximum for the particle emitters.
	if (maxInstanceValues > 0 && (!drawer->instanceValues ||
			maxInstanceValues > dsSharedMaterialValues_getMaxValues(drawer->instanceValues)))
	{
		dsSharedMaterialValues_destroy(drawer->instanceValues);
		drawer->instanceValues =
			dsSharedMaterialValues_create(drawer->allocator, maxInstanceValues);
		if (!drawer->instanceValues)
			DS_PROFILE_FUNC_RETURN(false);
	}

	// Make sure we have enough storage for the particle data. Use max particles to reach a steady
	// state sooner.
	if (maxParticles > drawer->maxParticles)
	{
		ParticleRef* newParticles = (ParticleRef*)dsAllocator_reallocWithFallback(drawer->allocator,
			drawer->particles, 0, maxParticles);
		if (!newParticles)
			DS_PROFILE_FUNC_RETURN(false);

		drawer->particles = newParticles;
		drawer->maxParticles = maxParticles;
	}

	// Get the buffer data.
	BufferInfo* bufferInfo = getDrawBuffer(drawer, particleCount, maxParticles);
	if (!bufferInfo)
		DS_PROFILE_FUNC_RETURN(false);

	// Draw the particles to the command buffer.
	collectParticles(drawer, viewMatrix, emitters, emitterCount, particleCount);

	bool success = populateParticleGeometry(drawer, bufferInfo, particleCount) &&
		drawParticles(drawer, emitters, emitterCount, bufferInfo, particleCount, commandBuffer,
			globalValues, drawData);

	DS_PROFILE_FUNC_RETURN(success);
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
	DS_VERIFY(dsAllocator_free(drawer->allocator, drawer->particles));
	DS_VERIFY(dsAllocator_free(drawer->allocator, drawer));
	return true;
}
