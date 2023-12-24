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

#include <DeepSea/Physics/Shapes/PhysicsCone.h>

#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Physics/Types.h>

static dsPhysicsShapeType coneType = {.staticBodiesOnly = false, .uniformScaleOnly = true};
const dsPhysicsShapeType* dsPhysicsCone_type(void)
{
	return &coneType;
}

dsPhysicsCone* dsPhysicsCone_create(dsPhysicsEngine* engine, dsAllocator* allocator,
	float halfHeight, float radius, dsPhysicsAxis axis, float convexRadius)
{
	if (!engine || !engine->createConeFunc || !engine->destroyConeFunc || axis < dsPhysicsAxis_X ||
		axis > dsPhysicsAxis_Z)
	{
		errno = EINVAL;
		return NULL;
	}

	if (halfHeight <= 0)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Cone half height must be > 0.");
		errno = EINVAL;
		return NULL;
	}

	if (radius <= 0)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Cone radius must be > 0.");
		errno = EINVAL;
		return NULL;
	}

	if (convexRadius < 0)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Cone convex radius must be >= 0.");
		errno = EINVAL;
		return NULL;
	}

	if (!allocator)
		allocator = engine->allocator;

	return engine->createConeFunc(engine, allocator, halfHeight, radius, axis, convexRadius);
}

bool dsPhysicsCone_destroy(dsPhysicsCone* cone)
{
	if (!cone)
		return true;

	dsPhysicsEngine* engine = ((dsPhysicsShape*)cone)->engine;
	if (!engine || !engine->destroyConeFunc)
	{
		errno = EINVAL;
		return false;
	}

	return engine->destroyConeFunc(engine, cone);
}
