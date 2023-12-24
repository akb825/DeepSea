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

#include <DeepSea/Physics/Shapes/PhysicsSphere.h>

#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Physics/Types.h>

static dsPhysicsShapeType sphereType = {.staticBodiesOnly = false, .uniformScaleOnly = true};
const dsPhysicsShapeType* dsPhysicsSphere_type(void)
{
	return &sphereType;
}

dsPhysicsSphere* dsPhysicsSphere_create(dsPhysicsEngine* engine, dsAllocator* allocator,
	float radius)
{
	if (!engine || !engine->createSphereFunc || !engine->destroySphereFunc)
	{
		errno = EINVAL;
		return NULL;
	}

	if (radius <= 0)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Sphere radius must be > 0.");
		errno = EINVAL;
		return NULL;
	}

	if (!allocator)
		allocator = engine->allocator;

	return engine->createSphereFunc(engine, allocator, radius);
}

bool dsPhysicsSphere_destroy(dsPhysicsSphere* sphere)
{
	if (!sphere)
		return true;

	dsPhysicsEngine* engine = ((dsPhysicsShape*)sphere)->engine;
	if (!engine || !engine->destroySphereFunc)
	{
		errno = EINVAL;
		return false;
	}

	return engine->destroySphereFunc(engine, sphere);
}
