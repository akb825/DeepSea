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

#include <DeepSea/SceneParticle/SceneStandardParticleEmitterFactory.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Random.h>

#include <DeepSea/Particle/ParticleEmitter.h>
#include <DeepSea/Particle/StandardParticleEmitter.h>

#include <DeepSea/Scene/Nodes/SceneTreeNode.h>
#include <DeepSea/SceneParticle/SceneParticleEmitterFactory.h>

#include <string.h>

typedef struct dsSceneStandardParticleEmitterFactory
{
	dsAllocator* allocator;
	dsParticleEmitterParams params;
	dsStandardParticleEmitterOptions options;
	const dsSceneNode* relativeNode;
	uint32_t seed;
	bool enabled;
	float startTime;
} dsSceneStandardParticleEmitterFactory;

static const dsMatrix44f* findRelativeTransform(const dsSceneTreeNode* treeNode,
	const dsSceneNode* relativeNode)
{
	if (!relativeNode)
		return NULL;

	do
	{
		const dsSceneTreeNode* parent = dsSceneTreeNode_getParent(treeNode);
		if (!parent)
			return NULL;
		else if (dsSceneTreeNode_getNode(parent) == relativeNode)
			return dsSceneTreeNode_getTransform(parent);
		treeNode = parent;
	} while (true);
}

static void computeVolumeMatrix(dsMatrix44f* outMatrix, const dsMatrix44f* transform,
	const dsMatrix44f* relativeTransform, const dsMatrix44f* volumeMatrix)
{
	dsMatrix44f relativeInverse, relativeToTransform;
	dsMatrix44f_affineInvert(&relativeInverse, relativeTransform);
	dsMatrix44_mul(relativeToTransform, relativeInverse, *transform);
	dsMatrix44_mul(*outMatrix, relativeToTransform, *volumeMatrix);
}

static dsParticleEmitter* dsSceneStandardParticleEmitterFactory_createEmitter(
	const dsSceneParticleNode* particleNode, dsAllocator* allocator, void* userData,
	const dsSceneTreeNode* treeNode)
{
	dsSceneStandardParticleEmitterFactory* factory =
		(dsSceneStandardParticleEmitterFactory*)userData;
	DS_UNUSED(particleNode);
	if (!particleNode || !allocator || !factory || !treeNode)
	{
		errno = EINVAL;
		return NULL;
	}

	const dsMatrix44f* transform = dsSceneTreeNode_getTransform(treeNode);
	DS_ASSERT(transform);
	const dsMatrix44f* relativeTransform = findRelativeTransform(treeNode, factory->relativeNode);

	// TODO: Set populate instance values based on node item lists.
	dsStandardParticleEmitter* standardEmitter = dsStandardParticleEmitter_create(allocator,
		&factory->params, factory->seed, &factory->options, factory->enabled, factory->startTime);

	// Advance the seed. (don't care about the actual value)
	dsRandom(&factory->seed);
	if (!standardEmitter)
		return NULL;

	dsParticleEmitter* emitter = (dsParticleEmitter*)standardEmitter;
	if (relativeTransform)
	{
		dsStandardParticleEmitterOptions* options =
			dsStandardParticleEmitter_getMutableOptions(standardEmitter);
		DS_ASSERT(options);
		computeVolumeMatrix(&options->volumeMatrix, transform, relativeTransform,
			&factory->options.volumeMatrix);
		emitter->transform = *relativeTransform;
	}
	else
		emitter->transform = *transform;

	return emitter;
}

static bool dsSceneStandardParticleEmitterFactory_updateEmitter(
	const dsSceneParticleNode* particleNode, void* userData, dsParticleEmitter* emitter,
	const dsSceneTreeNode* treeNode, float time)
{
	DS_UNUSED(particleNode);
	dsSceneStandardParticleEmitterFactory* factory =
		(dsSceneStandardParticleEmitterFactory*)userData;
	if (!factory || !emitter || !treeNode)
	{
		errno = EINVAL;
		return false;
	}

	const dsMatrix44f* transform = dsSceneTreeNode_getTransform(treeNode);
	DS_ASSERT(transform);
	// NOTE: Could cache this, but would take extra effort to wrap the particle emitter. Expected
	// to be at most a couple of nodes up, so don't expect a large impact on performance.
	const dsMatrix44f* relativeTransform = findRelativeTransform(treeNode, factory->relativeNode);

	dsStandardParticleEmitter* standardEmitter = (dsStandardParticleEmitter*)emitter;
	dsStandardParticleEmitterOptions* options =
		dsStandardParticleEmitter_getMutableOptions(standardEmitter);
	DS_ASSERT(options);
	if (relativeTransform)
	{
		computeVolumeMatrix(&options->volumeMatrix, transform, relativeTransform,
			&options->volumeMatrix);
		emitter->transform = *relativeTransform;
	}
	else
	{
		options->volumeMatrix = factory->options.volumeMatrix;
		emitter->transform = *transform;
	}

	return dsParticleEmitter_update(emitter, time);
}

static void dsSceneStandardParticleEmitterFactory_destroy(void* userData)
{
	dsSceneStandardParticleEmitterFactory* factory =
		(dsSceneStandardParticleEmitterFactory*)userData;
	if (factory && factory->allocator)
		DS_VERIFY(dsAllocator_free(factory->allocator, factory));
}

const char* const dsSceneStandardParticleEmitterFactory_typeName = "StandardParticleEmitter";

dsSceneParticleEmitterFactory* dsSceneStandardParticleEmitterFactory_create(
	dsAllocator* allocator, const dsParticleEmitterParams* params, uint32_t seed,
	const dsStandardParticleEmitterOptions* options, bool enabled, float startTime,
	const dsSceneNode* relativeNode)
{
	if (!allocator || !params || !options)
	{
		errno = EINVAL;
		return NULL;
	}

	dsSceneStandardParticleEmitterFactory* data =
		DS_ALLOCATE_OBJECT(allocator, dsSceneStandardParticleEmitterFactory);
	if (!data)
		return NULL;

	data->allocator = dsAllocator_keepPointer(allocator);
	memcpy(&data->params, params, sizeof(dsParticleEmitterParams));
	memcpy(&data->options, options, sizeof(dsStandardParticleEmitterOptions));
	data->relativeNode = relativeNode;
	data->seed = seed;
	data->enabled = enabled;
	data->startTime = startTime;

	return dsSceneParticleEmitterFactory_create(allocator,
		&dsSceneStandardParticleEmitterFactory_createEmitter,
		&dsSceneStandardParticleEmitterFactory_updateEmitter, data,
		&dsSceneStandardParticleEmitterFactory_destroy);
}

const dsParticleEmitterParams* dsSceneParticleEmitterFactory_getEmitterParams(
	const dsSceneParticleEmitterFactory* factory)
{
	if (!factory || !factory->userData ||
		factory->createEmitterFunc != &dsSceneStandardParticleEmitterFactory_createEmitter)
	{
		errno = EINVAL;
		return NULL;
	}

	return &((dsSceneStandardParticleEmitterFactory*)factory->userData)->params;
}

dsParticleEmitterParams* dsSceneParticleEmitterFactory_getMutableEmitterParams(
	dsSceneParticleEmitterFactory* factory)
{
	if (!factory || !factory->userData ||
		factory->createEmitterFunc != &dsSceneStandardParticleEmitterFactory_createEmitter)
	{
		errno = EINVAL;
		return NULL;
	}

	return &((dsSceneStandardParticleEmitterFactory*)factory->userData)->params;
}

const dsStandardParticleEmitterOptions* dsSceneParticleEmitterFactory_getSandardOptions(
	const dsSceneParticleEmitterFactory* factory)
{
	if (!factory || !factory->userData ||
		factory->createEmitterFunc != &dsSceneStandardParticleEmitterFactory_createEmitter)
	{
		errno = EINVAL;
		return NULL;
	}

	return &((dsSceneStandardParticleEmitterFactory*)factory->userData)->options;
}

dsStandardParticleEmitterOptions* dsSceneParticleEmitterFactory_getMutableSandardOptions(
	dsSceneParticleEmitterFactory* factory)
{
	if (!factory || !factory->userData ||
		factory->createEmitterFunc != &dsSceneStandardParticleEmitterFactory_createEmitter)
	{
		errno = EINVAL;
		return NULL;
	}

	return &((dsSceneStandardParticleEmitterFactory*)factory->userData)->options;
}

uint32_t dsSceneStandardParticleEmitterFactory_getSeed(const dsSceneParticleEmitterFactory* factory)
{
	if (!factory || !factory->userData ||
		factory->createEmitterFunc != &dsSceneStandardParticleEmitterFactory_createEmitter)
	{
		return 0;
	}

	return ((dsSceneStandardParticleEmitterFactory*)factory->userData)->seed;
}

bool dsSceneStandardParticleEmitterFactory_setSeed(const dsSceneParticleEmitterFactory* factory,
	uint32_t seed)
{
	if (!factory || !factory->userData ||
		factory->createEmitterFunc != &dsSceneStandardParticleEmitterFactory_createEmitter)
	{
		errno = EINVAL;
		return false;
	}

	((dsSceneStandardParticleEmitterFactory*)factory->userData)->seed = seed;
	return true;
}

bool dsSceneStandardParticleEmitterFactory_getEnabled(
	const dsSceneParticleEmitterFactory* factory)
{
	if (!factory || !factory->userData ||
		factory->createEmitterFunc != &dsSceneStandardParticleEmitterFactory_createEmitter)
	{
		return false;
	}

	return ((dsSceneStandardParticleEmitterFactory*)factory->userData)->enabled;
}

bool dsSceneStandardParticleEmitterFactory_setEnabled(const dsSceneParticleEmitterFactory* factory,
	bool enabled)
{
	if (!factory || !factory->userData ||
		factory->createEmitterFunc != &dsSceneStandardParticleEmitterFactory_createEmitter)
	{
		errno = EINVAL;
		return false;
	}

	((dsSceneStandardParticleEmitterFactory*)factory->userData)->enabled = enabled;
	return true;
}

const dsSceneNode* dsSceneStandardParticleEmitterFactory_getRelativeNode(
	const dsSceneParticleEmitterFactory* factory)
{
	if (!factory || !factory->userData ||
		factory->createEmitterFunc != &dsSceneStandardParticleEmitterFactory_createEmitter)
	{
		return NULL;
	}

	return ((dsSceneStandardParticleEmitterFactory*)factory->userData)->relativeNode;
}

bool dsSceneStandardParticleEmitterFactory_setRelativeNode(
	const dsSceneParticleEmitterFactory* factory, const dsSceneNode* relativeNode)
{
	if (!factory || !factory->userData ||
		factory->createEmitterFunc != &dsSceneStandardParticleEmitterFactory_createEmitter)
	{
		errno = EINVAL;
		return false;
	}

	((dsSceneStandardParticleEmitterFactory*)factory->userData)->relativeNode = relativeNode;
	return true;
}