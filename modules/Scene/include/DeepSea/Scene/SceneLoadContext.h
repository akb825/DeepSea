/*
 * Copyright 2019 Aaron Barany
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
#include <DeepSea/Scene/Export.h>
#include <DeepSea/Scene/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating scene load contexts.
 * @see dsSceneLoadContext
 */

/**
 * @brief Gets the size of a dsSceneLoadContext.
 * @return sizeof(dsSceneLoadContext)
 */
DS_SCENE_EXPORT size_t dsSceneLoadContext_sizeof(void);

/**
 * @brief Gets the full allocation size of a dsSceneLoadContext.
 * @return The full allocation size of dsSceneLoadContext.
 */
DS_SCENE_EXPORT size_t dsSceneLoadContext_fullAllocSize(void);

/**
 * @brief Creates a scene load context.
 *
 * The types declared inside of the Scene library will be automtically be registered with the load
 * context.
 *
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the load context with.
 * @return The load context or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsSceneLoadContext* dsSceneLoadContext_create(dsAllocator* allocator);

/**
 * @brief Registers a node type that can be loaded.
 *
 * Up to 128 node types can be registered within a single dsSceneLoadContext. This is intended to be
 * enough to support any reasonable situation. If this isn't enough for specialized situations,
 * multiple load contexts can be maintained to load different files with a subset of the types.
 *
 * @remark errno will be set on failure.
 * @param context The context to register the type with.
 * @param name The name of the type. The length, including the null terminator, must not exceed
 *     DS_MAX_SCENE_NAME_LENGTH.
 * @param loadFunc The function to call to perform the load.
 * @param userData The user data associated with the type. Any modifications made to this should be
 *     thread-safe if the same dsSceneLoadContext is used across multiple threads. This may be NULL.
 * @param destroyUserDataFunc The function to destroy the user data. This may be NULL.
 * @return False if the type couldn't be registered.
 */
DS_SCENE_EXPORT bool dsSceneLoadContext_registerNodeType(dsSceneLoadContext* context,
	const char* name, dsLoadSceneNodeFunction loadFunc, void* userData,
	dsDestroySceneUserDataFunction destroyUserDataFunc);

/**
 * @brief Registers an item list type that can be loaded.
 *
 * Up to 128 item list types can be registered within a single dsSceneLoadContext. This is intended
 * to be enough to support any reasonable situation. If this isn't enough for specialized
 * situations, multiple load contexts can be maintained to load different files with a subset of the
 * types.
 *
 * @remark errno will be set on failure.
 * @param context The context to register the type with.
 * @param name The name of the type. The length, including the null terminator, must not exceed
 *     DS_MAX_SCENE_NAME_LENGTH.
 * @param loadFunc The function to call to perform the load.
 * @param userData The user data associated with the type. Any modifications made to this should be
 *     thread-safe if the same dsSceneLoadContext is used across multiple threads. This may be NULL.
 * @param destroyUserDataFunc The function to destroy the user data. This may be NULL.
 * @return False if the type couldn't be registered.
 */
DS_SCENE_EXPORT bool dsSceneLoadContext_registerItemListType(dsSceneLoadContext* context,
	const char* name, dsLoadSceneItemListFunction loadFunc, void* userData,
	dsDestroySceneUserDataFunction destroyUserDataFunc);

/**
 * @brief Registers a global data type that can be loaded.
 *
 * Up to 128 global data types can be registered within a single dsSceneLoadContext. This is
 * intended to be enough to support any reasonable situation. If this isn't enough for specialized
 * situations, multiple load contexts can be maintained to load different files with a subset of the
 * types.
 *
 * @remark errno will be set on failure.
 * @param context The context to register the type with.
 * @param name The name of the type. The length, including the null terminator, must not exceed
 *     DS_MAX_SCENE_NAME_LENGTH.
 * @param loadFunc The function to call to perform the load.
 * @param userData The user data associated with the type. Any modifications made to this should be
 *     thread-safe if the same dsSceneLoadContext is used across multiple threads. This may be NULL.
 * @param destroyUserDataFunc The function to destroy the user data. This may be NULL.
 * @return False if the type couldn't be registered.
 */
DS_SCENE_EXPORT bool dsSceneLoadContext_registerGlobalDataType(dsSceneLoadContext* context,
	const char* name, dsLoadSceneGlobalDataFunction loadFunc, void* userData,
	dsDestroySceneUserDataFunction destroyUserDataFunc);

/**
 * @brief Destroys a scene load context.
 * @param context The context to destroy.
 */
DS_SCENE_EXPORT void dsSceneLoadContext_destroy(dsSceneLoadContext* context);

#ifdef __cplusplus
}
#endif
