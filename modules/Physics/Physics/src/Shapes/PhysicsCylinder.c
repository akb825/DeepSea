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

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Physics/Shapes/PhysicsShape.h>
#include <DeepSea/Physics/PhysicsMassProperties.h>
#include <DeepSea/Physics/Types.h>

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
	.getMassPropertiesFunc = dsPhysicsCylinder_getMassProperties, .getMaterialFunc = NULL};
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

	return engine->createCylinderFunc(engine, allocator, halfHeight, radius, axis, convexRadius);
}

void dsPhysicsCylinder_initialize(dsPhysicsCylinder* cylinder, dsPhysicsEngine* engine,
	dsAllocator* allocator, void* impl, float halfHeight, float radius, dsPhysicsAxis axis,
	float convexRadius)
{
	DS_ASSERT(cylinder);
	DS_ASSERT(engine);
	DS_ASSERT(halfHeight > 0);
	DS_ASSERT(radius > 0);
	DS_ASSERT(axis >= dsPhysicsAxis_X && axis <= dsPhysicsAxis_Z);

	dsAlignedBox3f bounds;
	switch (axis)
	{
		case dsPhysicsAxis_X:
			bounds.min.x = -halfHeight;
			bounds.min.y = bounds.min.z = -radius;
			bounds.max.x = halfHeight;
			bounds.max.y = bounds.max.z = radius;
			break;
		case dsPhysicsAxis_Y:
			bounds.min.y = -halfHeight;
			bounds.min.x = bounds.min.z = -radius;
			bounds.max.y = halfHeight;
			bounds.max.x = bounds.max.z = radius;
			break;
		case dsPhysicsAxis_Z:
			bounds.min.z = -halfHeight;
			bounds.min.x = bounds.min.y = -radius;
			bounds.max.z = halfHeight;
			bounds.max.x = bounds.max.y = radius;
			break;
	}

	DS_VERIFY(dsPhysicsShape_initialize((dsPhysicsShape*)cylinder, engine, allocator,
		dsPhysicsCylinder_type(), &bounds, impl,
		(dsDestroyPhysicsShapeFunction)engine->destroyCylinderFunc));

	cylinder->halfHeight = halfHeight;
	cylinder->radius = radius;
	cylinder->axis = axis;
	cylinder->convexRadius = convexRadius;
}
