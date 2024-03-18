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

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Physics/Shapes/PhysicsShape.h>
#include <DeepSea/Physics/PhysicsMassProperties.h>
#include <DeepSea/Physics/Types.h>

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
	.getMassPropertiesFunc = &dsPhysicsCapsule_getMassProperties, .getMaterialFunc = NULL};
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

	return engine->createCapsuleFunc(engine, allocator, halfHeight, radius, axis);
}

void dsPhysicsCapsule_initialize(dsPhysicsCapsule* capsule, dsPhysicsEngine* engine,
	dsAllocator* allocator, void* impl, float halfHeight, float radius, dsPhysicsAxis axis)
{
	DS_ASSERT(capsule);
	DS_ASSERT(engine);
	DS_ASSERT(halfHeight > 0);
	DS_ASSERT(radius > 0);
	DS_ASSERT(axis >= dsPhysicsAxis_X && axis <= dsPhysicsAxis_Z);

	dsAlignedBox3f bounds;
	switch (axis)
	{
		case dsPhysicsAxis_X:
			bounds.min.x = -halfHeight - radius;
			bounds.min.y = bounds.min.z = -radius;
			bounds.max.x = halfHeight + radius;
			bounds.max.y = bounds.max.z = radius;
			break;
		case dsPhysicsAxis_Y:
			bounds.min.y = -halfHeight - radius;
			bounds.min.x = bounds.min.z = -radius;
			bounds.max.y = halfHeight + radius;
			bounds.max.x = bounds.max.z = radius;
			break;
		case dsPhysicsAxis_Z:
			bounds.min.z = -halfHeight - radius;
			bounds.min.x = bounds.min.y = -radius;
			bounds.max.z = halfHeight + radius;
			bounds.max.x = bounds.max.y = radius;
			break;
	}

	DS_VERIFY(dsPhysicsShape_initialize((dsPhysicsShape*)capsule, engine, allocator,
		dsPhysicsCapsule_type(), &bounds, impl,
		(dsDestroyPhysicsShapeFunction)engine->destroyCapsuleFunc));

	capsule->halfHeight = halfHeight;
	capsule->radius = radius;
	capsule->axis = axis;
}
