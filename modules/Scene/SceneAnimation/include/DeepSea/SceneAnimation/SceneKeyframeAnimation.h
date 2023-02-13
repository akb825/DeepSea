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

#pragma once

#include <DeepSea/Core/Config.h>

#include <DeepSea/Animation/Types.h>
#include <DeepSea/Scene/Types.h>
#include <DeepSea/SceneAnimation/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Function for registering dsKeyframeAnimation with dsSceneResources.
 */

/**
 * @brief The type name for a scene keyframe animation.
 */
DS_SCENEANIMATION_EXPORT extern const char* const dsSceneKeyframeAnimation_typeName;

/**
 * @brief Gets the type for the dsKeyframeAnimation custom type for storage in dsSceneResources.
 * @return The custom type.
 */
DS_SCENEANIMATION_EXPORT const dsCustomSceneResourceType* dsSceneKeyframeAnimation_type(void);

/**
 * @brief Creates a custom resource to wrap a dsKeyframeAnimation.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the custom resource.
 * @param animation The keyframe animation to wrap.
 * @return The custom resource or NULL if an error occurred.
 */
DS_SCENEANIMATION_EXPORT dsCustomSceneResource* dsSceneKeyframeAnimation_create(
	dsAllocator* allocator, dsKeyframeAnimation* animation);

/**
 * @brief Destroys a keyframe animation within a resource.
 * @param animation The keyframe animation to destroy.
 * @return False if the keyframe animation couldn't be destroyed
 */
DS_SCENEANIMATION_EXPORT bool dsSceneKeyframeAnimation_destroy(void* animation);

#ifdef __cplusplus
}
#endif
