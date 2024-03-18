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

#include <DeepSea/Physics/Shapes/PhysicsBox.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Vector3.h>

#include <DeepSea/Physics/Shapes/PhysicsShape.h>
#include <DeepSea/Physics/PhysicsMassProperties.h>
#include <DeepSea/Physics/Types.h>

static bool dsPhysicsBox_getMassProperties(dsPhysicsMassProperties* outMassProperties,
	const dsPhysicsShape* shape, float density)
{
	DS_ASSERT(outMassProperties);
	DS_ASSERT(shape);
	DS_ASSERT(shape->type == dsPhysicsBox_type());
	DS_ASSERT(density > 0);

	const dsPhysicsBox* box = (const dsPhysicsBox*)shape;
	dsVector3f halfExtents = box->halfExtents;
	// Make no extent is zero.
	halfExtents.x = dsMax(halfExtents.x, box->convexRadius);
	halfExtents.x = dsMax(halfExtents.y, box->convexRadius);
	halfExtents.x = dsMax(halfExtents.z, box->convexRadius);
	return dsPhysicsMassProperties_initializeBox(outMassProperties, &halfExtents, density);
}

static dsPhysicsShapeType boxType = {.staticBodiesOnly = false, .uniformScaleOnly = false,
	.getMassPropertiesFunc = &dsPhysicsBox_getMassProperties, .getMaterialFunc = NULL};
const dsPhysicsShapeType* dsPhysicsBox_type(void)
{
	return &boxType;
}

dsPhysicsBox* dsPhysicsBox_create(dsPhysicsEngine* engine, dsAllocator* allocator,
	const dsVector3f* halfExtents, float convexRadius)
{
	if (!engine || !engine->createBoxFunc || !engine->destroyBoxFunc || !halfExtents)
	{
		errno = EINVAL;
		return NULL;
	}

	if (halfExtents->x < 0 || halfExtents->y < 0 || halfExtents->z < 0)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Each box half extents axis must be >= 0.");
		errno = EINVAL;
		return NULL;
	}

	if (convexRadius < 0)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Box convex radius must be >= 0.");
		errno = EINVAL;
		return NULL;
	}

	if (!allocator)
		allocator = engine->allocator;

	return engine->createBoxFunc(engine, allocator, halfExtents, convexRadius);
}

void dsPhysicsBox_initialize(dsPhysicsBox* box, dsPhysicsEngine* engine, dsAllocator* allocator,
	void* impl, const dsVector3f* halfExtents, float convexRadius)
{
	DS_ASSERT(box);
	DS_ASSERT(engine);
	DS_ASSERT(halfExtents && halfExtents->x >= 0 && halfExtents->y >= 0 && halfExtents->z >= 0);
	DS_ASSERT(convexRadius >= 0);

	dsAlignedBox3f bounds;
	dsVector3_neg(bounds.min, *halfExtents);
	bounds.max = *halfExtents;
	DS_VERIFY(dsPhysicsShape_initialize((dsPhysicsShape*)box, engine, allocator,
		dsPhysicsBox_type(), &bounds, impl, (dsDestroyPhysicsShapeFunction)engine->destroyBoxFunc));

	box->halfExtents = *halfExtents;
	box->convexRadius = convexRadius;
}
