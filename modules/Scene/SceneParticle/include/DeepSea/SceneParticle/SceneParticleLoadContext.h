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
#include <DeepSea/Scene/Types.h>
#include <DeepSea/SceneParticle/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Function for registering the SceneParticle types with a dsSceneLoadContext.
 */

/**
 * @brief Registers the scene particle types for loading.
 * @remark errno will be set on failure.
 * @param loadContext The load context to register the types with.
 * @return False if not all of the types could be registered.
 */
DS_SCENEPARTICLE_EXPORT bool dsSceneParticleLoadConext_registerTypes(
	dsSceneLoadContext* loadContext);

#ifdef __cplusplus
}
#endif

