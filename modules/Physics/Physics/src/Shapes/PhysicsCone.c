/*
 * Copyright 2023-2024 Aaron Barany
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

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Physics/Types.h>
#include <DeepSea/Physics/PhysicsMassProperties.h>

static bool dsPhysicsCone_getMassProperties(dsPhysicsMassProperties* outMassProperties,
	const dsPhysicsShape* shape, float density)
{
	DS_ASSERT(outMassProperties);
	DS_ASSERT(shape);
	DS_ASSERT(shape->type == dsPhysicsCone_type());
	DS_ASSERT(density > 0);

	const dsPhysicsCone* cone = (const dsPhysicsCone*)shape;
	return dsPhysicsMassProperties_initializeCone(outMassProperties, cone->height,
		cone->radius, cone->axis, density);
}

static dsPhysicsShapeType coneType = {.staticBodiesOnly = false, .uniformScaleOnly = true,
	.getMassPropertiesFunc = &dsPhysicsCone_getMassProperties, .getMaterialFunc = NULL};
const dsPhysicsShapeType* dsPhysicsCone_type(void)
{
	return &coneType;
}

dsPhysicsCone* dsPhysicsCone_create(dsPhysicsEngine* engine, dsAllocator* allocator,
	float height, float radius, dsPhysicsAxis axis, float convexRadius)
{
	if (!engine || !engine->createConeFunc || !engine->destroyConeFunc || axis < dsPhysicsAxis_X ||
		axis > dsPhysicsAxis_Z)
	{
		errno = EINVAL;
		return NULL;
	}

	if (height <= 0)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Cone height must be > 0.");
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

	dsPhysicsCone* cone = engine->createConeFunc(
		engine, allocator, height, radius, axis, convexRadius);
	if (!cone)
		return NULL;

	dsPhysicsShape* shape = (dsPhysicsShape*)cone;
	switch (axis)
	{
		case dsPhysicsAxis_X:
			shape->bounds.min.x = 0.0f;
			shape->bounds.min.y = shape->bounds.min.z = -radius;
			shape->bounds.max.x = height;
			shape->bounds.max.y = shape->bounds.max.z = radius;
			break;
		case dsPhysicsAxis_Y:
			shape->bounds.min.y = 0.0f;
			shape->bounds.min.x = shape->bounds.min.z = -radius;
			shape->bounds.max.y = height;
			shape->bounds.max.x = shape->bounds.max.z = radius;
			break;
		case dsPhysicsAxis_Z:
			shape->bounds.min.z = 0.0f;
			shape->bounds.min.x = shape->bounds.min.y = -radius;
			shape->bounds.max.z = height;
			shape->bounds.max.x = shape->bounds.max.y = radius;
			break;
	}
	return cone;
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
