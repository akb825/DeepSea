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
#include <DeepSea/Core/Types.h>
#include <DeepSea/Scene/Export.h>
#include <DeepSea/Scene/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating views.
 * @see dsView
 */

/**
 * @brief Registers a cull ID with the cull manager owned by the view.
 *
 * This must be done as part of the synchronous item list in the scene. While this modifies the
 * state of view despite being const, this is temporary data that is allowed to be modified.
 *
 * @remark Other interactions with the cull manager should be done directly from the cullManager
 * member of dsView. This is provided to make it explicit that it's safe to perform this non-const
 * operation on a const dsView instance.
 *
 * @remark errno will be set on failure.
 * @param view The view to register the cull ID with.
 * @param cullID The cull ID to register.
 * @return The cull instance or DS_SCENE_NO_CULL if it cannot be registered.
 */
DS_SCENE_EXPORT uint32_t dsView_registerCullID(const dsView* view, dsSceneCullID cullID);

#ifdef __cplusplus
}
#endif

