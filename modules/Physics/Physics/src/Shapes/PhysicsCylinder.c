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

#include <DeepSea/Physics/Shapes/PhysicsCylinder.h>

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Physics/Types.h>
#include <DeepSea/Physics/PhysicsMassProperties.h>

static bool dsPhysicsCylinder_getMassProperties(dsPhysicsMassProperties* outMassProperties,
	const dsPhysicsShape* shape, float density)
{
	DS_ASSERT(outMassProperties);
	DS_ASSERT(shape);
	DS_ASSERT(shape->type == dsPhysicsCylinder_type());
	DS_ASSERT(density > 0);

	const dsPhysicsCylinder* cylinder = (const dsPhysicsCylinder*)shape;
	return dsPhysicsMassProperties_initializeCylinder(outMassProperties, cylinder->halfHeight,
		cylinder->radius, cylinder->axis, density);
}

static dsPhysicsShapeType cylinderType = {.staticBodiesOnly = false, .uniformScaleOnly = true,
	.getMassPropertiesFunc = dsPhysicsCylinder_getMassProperties};
const dsPhysicsShapeType* dsPhysicsCylinder_type(void)
{
	return &cylinderType;
}

dsPhysicsCylinder* dsPhysicsCylinder_create(dsPhysicsEngine* engine, dsAllocator* allocator,
	float halfHeight, float radius, dsPhysicsAxis axis, float convexRadius)
{
	if (!engine || !engine->createCylinderFunc || !engine->destroyConvexHullFunc ||
		axis < dsPhysicsAxis_X || axis > dsPhysicsAxis_Z)
	{
		errno = EINVAL;
		return NULL;
	}

	if (halfHeight <= 0)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Cylinder half height must be > 0.");
		errno = EINVAL;
		return NULL;
	}

	if (radius <= 0)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Cylinder radius must be > 0.");
		errno = EINVAL;
		return NULL;
	}

	if (convexRadius < 0)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Cylinder convex radius must be >= 0.");
		errno = EINVAL;
		return NULL;
	}

	if (!allocator)
		allocator = engine->allocator;

	dsPhysicsCylinder* cylinder = engine->createCylinderFunc(
		engine, allocator, halfHeight, radius, axis, convexRadius);
	if (!cylinder)
		return NULL;

	dsPhysicsShape* shape = (dsPhysicsShape*)cylinder;
	switch (axis)
	{
		case dsPhysicsAxis_X:
			shape->bounds.min.x = -halfHeight;
			shape->bounds.min.y = shape->bounds.min.z = -radius;
			shape->bounds.max.x = halfHeight;
			shape->bounds.max.y = shape->bounds.max.z = radius;
			break;
		case dsPhysicsAxis_Y:
			shape->bounds.min.y = -halfHeight;
			shape->bounds.min.x = shape->bounds.min.z = -radius;
			shape->bounds.max.y = halfHeight;
			shape->bounds.max.x = shape->bounds.max.z = radius;
			break;
		case dsPhysicsAxis_Z:
			shape->bounds.min.z = -halfHeight;
			shape->bounds.min.x = shape->bounds.min.y = -radius;
			shape->bounds.max.z = halfHeight;
			shape->bounds.max.x = shape->bounds.max.y = radius;
			break;
	}
	return cylinder;
}

bool dsPhysicsCylinder_destroy(dsPhysicsCylinder* cylinder)
{
	if (!cylinder)
		return true;

	dsPhysicsEngine* engine = ((dsPhysicsShape*)cylinder)->engine;
	if (!engine || !engine->destroyCylinderFunc)
	{
		errno = EINVAL;
		return false;
	}

	return engine->destroyCylinderFunc(engine, cylinder);
}
