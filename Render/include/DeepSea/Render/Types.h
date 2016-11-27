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
#include <DeepSea/Core/Thread/Types.h>
#include <DeepSea/Math/Types.h>
#include <DeepSea/Render/Resources/Types.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Includes all of the types used in the DeepSea/Render library.
 */

/**
 * @brief Log tag used by the render library.
 */
#define DS_RENDER_LOG_TAG "render"

/**
 * @brief Base object for interfacing with the DeepSea Render library.
 *
 * Render implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsRenderer and the true internal type.
 *
 * To ensure a lack of contention for system resources, only one dsRenderer instance should be used
 * in any given application.
 */
typedef struct dsRenderer
{
	/**
	 * @brief Manager for resources used with the renderer.
	 */
	dsResourceManager* resourceManager;

	/**
	 * @brief The main allocator for the Renderer library.
	 */
	dsAllocator* allocator;

	/**
	 * @brief Thread ID for the main thread.
	 *
	 * Some operations may only be done from the main thread.
	 */
	dsThreadId mainThread;
} dsRenderer;

#ifdef __cplusplus
}
#endif
