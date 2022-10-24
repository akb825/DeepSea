/*
 * Copyright 2019-2022 Aaron Barany
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
 * @param renderer The renderer to use for all graphics types created.
 * @return The load context or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsSceneLoadContext* dsSceneLoadContext_create(dsAllocator* allocator,
	dsRenderer* renderer);

/**
 * @brief Gets the renderer for a load context.
 * @remark errno will be set on failure.
 * @param context The load context.
 * @return The renderer, or NULL if context is NULL.
 */
DS_SCENE_EXPORT dsRenderer* dsSceneLoadContext_getRenderer(const dsSceneLoadContext* context);

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
 * @brief Registers a local data type that can be loaded.
 *
 * Up to 128 local data types can be registered within a single dsSceneLoadContext. This is
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
DS_SCENE_EXPORT bool dsSceneLoadContext_registerInstanceDataType(dsSceneLoadContext* context,
	const char* name, dsLoadSceneInstanceDataFunction loadFunc, void* userData,
	dsDestroySceneUserDataFunction destroyUserDataFunc);

/**
 * @brief Registers a custom resource type that can be loaded.
 *
 * Up to 128 custom resource types can be registered within a single dsSceneLoadContext. This is
 * intended to be enough to support any reasonable situation. If this isn't enough for specialized
 * situations, multiple load contexts can be maintained to load different files with a subset of the
 * types.
 *
 * @remark errno will be set on failure.
 * @param context The context to register the type with.
 * @param name The name of the type. The length, including the null terminator, must not exceed
 *     DS_MAX_SCENE_NAME_LENGTH.
 * @param type The type of the resource.
 * @param loadFunc The function to call to perform the load.
 * @param destroyResourceFunc The function to destroy resources of this type.
 * @param userData The user data associated with the type. Any modifications made to this should be
 *     thread-safe if the same dsSceneLoadContext is used across multiple threads. This may be NULL.
 * @param destroyUserDataFunc The function to destroy the user data. This may be NULL.
 * @param additionalResources The number of additional resources that will be added by the custom
 *     resource on load.
 * @return False if the type couldn't be registered.
 */
DS_SCENE_EXPORT bool dsSceneLoadContext_registerCustomResourceType(dsSceneLoadContext* context,
	const char* name, const dsCustomSceneResourceType* type,
	dsLoadCustomSceneResourceFunction loadFunc,
	dsDestroyCustomSceneResourceFunction destroyResourceFunc, void* userData,
	dsDestroySceneUserDataFunction destroyUserDataFunc, uint32_t additionalResources);

/**
 * @brief Gets the number of additional resources added for a custom resource.
 * @param context The context the type is registered with.
 * @param name The name of the custom resource type.
 * @return The number of additional resources.
 */
DS_SCENE_EXPORT uint32_t dsSceneLoadContext_getCustomResourceAdditionalResources(
	const dsSceneLoadContext* context, const char* name);

/**
 * @brief Registers a resource action type that can be loaded.
 *
 * A resource action performs an operation during load without adding resources of its own.
 *
 * Up to 128 resource action types can be registered within a single dsSceneLoadContext. This is
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
 * @param additionalResources The number of additional resources that will be added by the custom
 *     resource on load.
 * @return False if the type couldn't be registered.
 */
DS_SCENE_EXPORT bool dsSceneLoadContext_registerResourceActionType(dsSceneLoadContext* context,
	const char* name, dsLoadSceneResourceActionFunction loadFunc, void* userData,
	dsDestroySceneUserDataFunction destroyUserDataFunc, uint32_t additionalResources);

/**
 * @brief Gets the number of additional resources added for a resource action.
 * @param context The context the type is registered with.
 * @param name The name of the resource action type.
 * @return The number of additional resources.
 */
DS_SCENE_EXPORT uint32_t dsSceneLoadContext_getResourceActionAdditionalResources(
	const dsSceneLoadContext* context, const char* name);

/**
 * @brief Destroys a scene load context.
 * @param context The context to destroy.
 */
DS_SCENE_EXPORT void dsSceneLoadContext_destroy(dsSceneLoadContext* context);

#ifdef __cplusplus
}
#endif
