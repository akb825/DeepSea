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

#include <DeepSea/Physics/Shapes/PhysicsBox.h>

#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Physics/Types.h>

static dsPhysicsShapeType boxType = {.staticBodiesOnly = false, .uniformScaleOnly = false};
const dsPhysicsShapeType* dsPhysicsBox_type(void)
{
	return &boxType;
}

dsPhysicsBox* dsPhysicsBox_create(dsPhysicsEngine* engine, dsAllocator* allocator,
	const dsVector3f* halfExtents, float convexRadius)
{
	if (!engine || !engine->createBoxFunc || !halfExtents)
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
