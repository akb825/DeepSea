/*
 * Copyright 2022 Aaron Barany
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
#include <DeepSea/SceneParticle/Export.h>
#include <DeepSea/SceneParticle/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating scene particle draw lists.
 *
 * This will draw particles prepared with scene particle prepares.
 *
 * @remark Since the scene particle prepare list instances are responsible for creating the particle
 * emitter, it must appear earlier in the item list array than any scene particle draw lists.
 */

/**
 * @brief The scene particle draw list type name.
 */
DS_SCENEPARTICLE_EXPORT extern const char* const dsSceneParticleDrawList_typeName;

/**
 * @brief Gets the type of a scene particle draw list.
 * @return The type of a scene particle draw list.
 */
DS_SCENEPARTICLE_EXPORT dsSceneItemListType dsSceneParticleDrawList_type(void);

/**
 * @brief Creates a scene particle prepare draw list.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the list with.
 * @param name The name of the particle draw. This will be copied.
 * @param resourceManager The resource manager to create graphics resources with.
 * @param resourceAllocator The allocator to allocate graphics resources with. If NULL, allocator
 *     will be used.
 * @param instanceData The list of instance datas to use. The array will be copied, and this will
 *     take ownership of each instance data. The instances will be destroyed if an error occurrs.
 * @param instanceDataCount The number of instance datas.
 * @return The particle draw or NULL if an error occurred.
 */
DS_SCENEPARTICLE_EXPORT dsSceneItemList* dsSceneParticleDrawList_create(dsAllocator* allocator,
	const char* name, dsResourceManager* resourceManager, dsAllocator* resourceAllocator,
	dsSceneInstanceData* const* instanceData, uint32_t instanceDataCount);

#ifdef __cplusplus
}
#endif
