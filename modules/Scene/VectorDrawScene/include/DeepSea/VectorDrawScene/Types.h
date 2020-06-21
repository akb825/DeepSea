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
#include <DeepSea/Scene/Types.h>
#include <DeepSea/VectorDraw/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Includes all of the types used in the DeepSea/VectorDrawScene library.
 */

/**
 * @brief Log tag used by the vector draw scene library.
 */
#define DS_VECTOR_DRAW_SCENE_LOG_TAG "vectordraw-scene"

/**
 * @brief Struct describing text used within a scene.
 * @see SceneText.h
 */
typedef struct dsSceneText
{
	/**
	 * @brief The allocator this was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The text that has gone through initial processing, but not yet formatted.
	 */
	dsText* text;

	/**
	 * @brief The styles for the text.
	 */
	dsTextStyle* styles;

	/**
	 * @brief The number of styles for the text.
	 */
	uint32_t styleCount;
} dsSceneText;

#ifdef __cplusplus
}
#endif
