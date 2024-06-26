/*
 * Copyright 2024 Aaron Barany
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

#include <DeepSea/Physics/Shapes/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

dsPhysicsShape* dsPhysicsShape_fromFlatbufferShape(dsPhysicsEngine* engine, dsAllocator* allocator,
	const void* fbShapePtr, dsFindPhysicsShapeFunction findShapeFunc, void* findShapeUserData,
	const char* name);

dsPhysicsShape* dsPhysicsShape_loadImpl(dsPhysicsEngine* engine, dsAllocator* allocator,
	dsFindPhysicsShapeFunction findShapeFunc, void* findShapeUserData, const void* data,
	size_t size, const char* name);

#ifdef __cplusplus
}
#endif
