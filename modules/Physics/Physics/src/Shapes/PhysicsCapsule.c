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

#include <DeepSea/Physics/Shapes/PhysicsCapsule.h>

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Physics/Types.h>
#include <DeepSea/Physics/PhysicsMassProperties.h>

static bool dsPhysicsCapsule_getMassProperties(dsPhysicsMassProperties* outMassProperties,
	const dsPhysicsShape* shape, float density)
{
	DS_ASSERT(outMassProperties);
	DS_ASSERT(shape);
	DS_ASSERT(shape->type == dsPhysicsCapsule_type());
	DS_ASSERT(density > 0);

	const dsPhysicsCapsule* capsule = (const dsPhysicsCapsule*)shape;
	return dsPhysicsMassProperties_initializeCapsule(outMassProperties, capsule->halfHeight,
		capsule->radius, capsule->axis, density);
}

static dsPhysicsShapeType capsuleType = {.staticBodiesOnly = false, .uniformScaleOnly = true,
	.getMassPropertiesFunc = &dsPhysicsCapsule_getMassProperties};
const dsPhysicsShapeType* dsPhysicsCapsule_type(void)
{
	return &capsuleType;
}

dsPhysicsCapsule* dsPhysicsCapsule_create(dsPhysicsEngine* engine, dsAllocator* allocator,
	float halfHeight, float radius, dsPhysicsAxis axis)
{
	if (!engine || !engine->createCapsuleFunc || !engine->destroyCapsuleFunc ||
		axis < dsPhysicsAxis_X || axis > dsPhysicsAxis_Z)
	{
		errno = EINVAL;
		return NULL;
	}

	if (halfHeight <= 0)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Capsule half height must be > 0.");
		errno = EINVAL;
		return NULL;
	}

	if (radius <= 0)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Capsule radius must be > 0.");
		errno = EINVAL;
		return NULL;
	}

	if (!allocator)
		allocator = engine->allocator;

	dsPhysicsCapsule* capsule =
		engine->createCapsuleFunc(engine, allocator, halfHeight, radius, axis);
	if (!capsule)
		return NULL;

	dsPhysicsShape* shape = (dsPhysicsShape*)capsule;
	switch (axis)
	{
		case dsPhysicsAxis_X:
			shape->bounds.min.x = -halfHeight - radius;
			shape->bounds.min.y = shape->bounds.min.z = -radius;
			shape->bounds.max.x = halfHeight + radius;
			shape->bounds.max.y = shape->bounds.max.z = radius;
			break;
		case dsPhysicsAxis_Y:
			shape->bounds.min.y = -halfHeight - radius;
			shape->bounds.min.x = shape->bounds.min.z = -radius;
			shape->bounds.max.y = halfHeight + radius;
			shape->bounds.max.x = shape->bounds.max.z = radius;
			break;
		case dsPhysicsAxis_Z:
			shape->bounds.min.z = -halfHeight - radius;
			shape->bounds.min.x = shape->bounds.min.y = -radius;
			shape->bounds.max.z = halfHeight + radius;
			shape->bounds.max.x = shape->bounds.max.y = radius;
			break;
	}
	return capsule;
}

bool dsPhysicsCapsule_destroy(dsPhysicsCapsule* capsule)
{
	if (!capsule)
		return true;

	dsPhysicsEngine* engine = ((dsPhysicsShape*)capsule)->engine;
	if (!engine || !engine->destroyCapsuleFunc)
	{
		errno = EINVAL;
		return false;
	}

	return engine->destroyCapsuleFunc(engine, capsule);
}
