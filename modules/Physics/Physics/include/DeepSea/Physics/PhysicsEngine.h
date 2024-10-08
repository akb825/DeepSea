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

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Physics/Export.h>
#include <DeepSea/Physics/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Function for manipulating physics engines.
 */

/**
 * @brief Destroys a physics engine.
 * @remark errno will be set on failure.
 * @param engine The physics engine to destroy.
 * @return False if the physics engine couldn't be destroyed.
 */
DS_PHYSICS_EXPORT bool dsPhysicsEngine_destroy(dsPhysicsEngine* engine);

#ifdef __cplusplus
}
#endif
