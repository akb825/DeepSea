/*
 * Copyright 2023 Aaron Barany
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

#include <DeepSea/Physics/PhysicsScene.h>
#include <DeepSea/Core/Error.h>

dsPhysicsScene* dsPhysicsScene_create(dsPhysicsEngine* engine, dsAllocator* allocator,
	const dsPhysicsSceneLimits* limits)
{
	if (!engine || (!allocator && !engine->allocator) || !limits || !engine->createSceneFunc ||
		!engine->destroySceneFunc)
	{
		errno = EINVAL;
		return NULL;
	}

	return engine->createSceneFunc(engine, allocator, limits);
}

bool dsPhysicsScene_destroy(dsPhysicsScene* scene)
{
	if (!scene)
		return true;

	if (!scene->engine || !scene->engine->destroySceneFunc)
	{
		errno = EINVAL;
		return false;
	}

	return scene->engine->destroySceneFunc(scene->engine, scene);
}
