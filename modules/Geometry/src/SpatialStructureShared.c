/*
 * Copyright 2018 Aaron Barany
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

#include "SpatialStructureShared.h"
#include <DeepSea/Geometry/AlignedBox2.h>
#include <DeepSea/Geometry/AlignedBox3.h>

const void* dsSpatialStructure_getObject(const void* objects, size_t objectSize, uint32_t index);

uint8_t dsSpatialStructure_maxAxis2f(const void* bounds)
{
	const dsAlignedBox2f* realBounds = (const dsAlignedBox2f*)bounds;
	dsVector2f extents;
	dsAlignedBox2_extents(extents, *realBounds);
	if (extents.x >= extents.y)
		return 0;
	return 1;
}

uint8_t dsSpatialStructure_maxAxis3f(const void* bounds)
{
	const dsAlignedBox3f* realBounds = (const dsAlignedBox3f*)bounds;
	dsVector3f extents;
	dsAlignedBox3_extents(extents, *realBounds);
	if (extents.x >= extents.y && extents.x >= extents.z)
		return 0;
	if (extents.y >= extents.x && extents.y >= extents.z)
		return 1;
	return 2;
}

uint8_t dsSpatialStructure_maxAxis2d(const void* bounds)
{
	const dsAlignedBox2d* realBounds = (const dsAlignedBox2d*)bounds;
	dsVector2d extents;
	dsAlignedBox2_extents(extents, *realBounds);
	if (extents.x >= extents.y)
		return 0;
	return 1;
}

uint8_t dsSpatialStructure_maxAxis3d(const void* bounds)
{
	const dsAlignedBox3d* realBounds = (const dsAlignedBox3d*)bounds;
	dsVector3d extents;
	dsAlignedBox3_extents(extents, *realBounds);
	if (extents.x >= extents.y && extents.x >= extents.z)
		return 0;
	if (extents.y >= extents.x && extents.y >= extents.z)
		return 1;
	return 2;
}

uint8_t dsSpatialStructure_maxAxis2i(const void* bounds)
{
	const dsAlignedBox2i* realBounds = (const dsAlignedBox2i*)bounds;
	dsVector2i extents;
	dsAlignedBox2_extents(extents, *realBounds);
	if (extents.x >= extents.y)
		return 0;
	return 1;
}

uint8_t dsSpatialStructure_maxAxis3i(const void* bounds)
{
	const dsAlignedBox3i* realBounds = (const dsAlignedBox3i*)bounds;
	dsVector3i extents;
	dsAlignedBox3_extents(extents, *realBounds);
	if (extents.x >= extents.y && extents.x >= extents.z)
		return 0;
	if (extents.y >= extents.x && extents.y >= extents.z)
		return 1;
	return 2;
}
