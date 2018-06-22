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

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Geometry/Types.h>

inline const void* dsSpatialStructure_getObject(const void* objects, size_t objectSize,
	uint32_t index)
{
	if (objectSize == DS_GEOMETRY_OBJECT_POINTERS)
		return ((const void**)objects)[index];
	else if (objectSize == DS_GEOMETRY_OBJECT_INDICES)
		return (const void*)(size_t)index;
	return ((const uint8_t*)objects) + objectSize*index;
}

typedef uint8_t (*MaxAxisFunction)(const void* bounds);
uint8_t dsSpatialStructure_maxAxis2f(const void* bounds);
uint8_t dsSpatialStructure_maxAxis3f(const void* bounds);
uint8_t dsSpatialStructure_maxAxis2d(const void* bounds);
uint8_t dsSpatialStructure_maxAxis3d(const void* bounds);
uint8_t dsSpatialStructure_maxAxis2i(const void* bounds);
uint8_t dsSpatialStructure_maxAxis3i(const void* bounds);
