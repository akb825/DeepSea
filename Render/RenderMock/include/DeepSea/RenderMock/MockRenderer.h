/*
 * Copyright 2016 Aaron Barany
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
#include <DeepSea/RenderMock/Export.h>
#include <DeepSea/Render/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and destroying a mock renderer.
 *
 * The mock renderer cannot be used to do actual rendering. Its intent is to be used for unit tests.
 */

/**
 * @brief Creates a mock renderer.
 * @param allocator The allocator to use.
 * @return The renderer.
 */
DS_RENDERMOCK_EXPORT dsRenderer* dsMockRenderer_create(dsAllocator* allocator);

/**
 * @brief Destroys a mock renderer.
 * @param renderer The renderer to destroy.
 */
DS_RENDERMOCK_EXPORT void dsMockRenderer_destroy(dsRenderer* renderer);

#ifdef __cplusplus
}
#endif
