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

#include <DeepSea/SceneParticle/SceneParticleEmitterFactory.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

static dsCustomSceneResourceType resourceType;
const dsCustomSceneResourceType* dsSceneParticleEmitterFactory_type(void)
{
	return &resourceType;
}

dsSceneParticleEmitterFactory* dsSceneParticleEmitterFactory_create(dsAllocator* allocator,
	dsCreateSceneParticleNodeEmitterFunction createEmitterFunc, void* userData,
	dsDestroySceneUserDataFunction destroyUserDataFunc)
{
	if (!allocator || !createEmitterFunc)
	{
		errno = EINVAL;
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		return NULL;
	}

	dsSceneParticleEmitterFactory* factory =
		DS_ALLOCATE_OBJECT(allocator, dsSceneParticleEmitterFactory);
	if (!factory)
	{
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		return NULL;
	}

	factory->allocator = dsAllocator_keepPointer(allocator);
	factory->createEmitterFunc = createEmitterFunc;
	factory->userData = userData;
	factory->destroyUserDataFunc = destroyUserDataFunc;

	return factory;
}

void dsSceneParticleEmitterFactory_destroy(dsSceneParticleEmitterFactory* factory)
{
	if (!factory)
		return;

	if (factory->destroyUserDataFunc)
		factory->destroyUserDataFunc(factory->userData);

	if (factory->allocator)
		DS_VERIFY(dsAllocator_free(factory->allocator, factory));
}
