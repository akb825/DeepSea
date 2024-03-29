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

#include <DeepSea/Physics/Shapes/PhysicsSphere.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Physics/Shapes/PhysicsShape.h>
#include <DeepSea/Physics/PhysicsMassProperties.h>
#include <DeepSea/Physics/Types.h>

static bool dsPhysicsSphere_getMassProperties(dsPhysicsMassProperties* outMassProperties,
	const dsPhysicsShape* shape, float density)
{
	DS_ASSERT(outMassProperties);
	DS_ASSERT(shape);
	DS_ASSERT(shape->type == dsPhysicsSphere_type());
	DS_ASSERT(density > 0);

	const dsPhysicsSphere* sphere = (const dsPhysicsSphere*)shape;
	return dsPhysicsMassProperties_initializeSphere(outMassProperties, sphere->radius, density);
}

static dsPhysicsShapeType sphereType = {.staticBodiesOnly = false, .uniformScaleOnly = true,
	.getMassPropertiesFunc = &dsPhysicsSphere_getMassProperties, .getMaterialFunc = NULL};
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

void dsPhysicsSphere_initialize(dsPhysicsSphere* sphere, dsPhysicsEngine* engine,
	dsAllocator* allocator, void* impl, float radius)
{
	DS_ASSERT(sphere);
	DS_ASSERT(engine);
	DS_ASSERT(radius > 0);

	dsAlignedBox3f bounds;
	bounds.min.x = bounds.min.y = bounds.min.z = -radius;
	bounds.max.x = bounds.max.y = bounds.max.z = radius;

	DS_VERIFY(dsPhysicsShape_initialize((dsPhysicsShape*)sphere, engine, allocator,
		dsPhysicsSphere_type(), &bounds, impl,
		(dsDestroyPhysicsShapeFunction)engine->destroySphereFunc));

	sphere->radius = radius;
}
