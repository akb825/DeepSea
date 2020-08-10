/*
 * Copyright 2020 Aaron Barany
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
#include <DeepSea/VectorDrawScene/Export.h>
#include <DeepSea/VectorDrawScene/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating scene vector prepare lists.
 *
 * This will prepare dsSceneVectorNode node types for drawing. This should be created as one of
 * the shared item lists for a scene since the preparation steps requires resource copying, which
 * must be done outside of a render pass.
 */

/**
 * @brief The scene vector prepare list type name.
 */
DS_VECTORDRAWSCENE_EXPORT extern const char* const dsSceneVectorPrepareList_typeName;

/**
 * @brief Creates a scene vector prepare list.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the list with. This must support freeing memory.
 * @param name The name of the vector prepare list. This will be copied.
 * @return The vector prepare list or NULL if an error occurred.
 */
DS_VECTORDRAWSCENE_EXPORT dsSceneItemList* dsSceneVectorPrepareList_create(dsAllocator* allocator,
	const char* name);

#ifdef __cplusplus
}
#endif
