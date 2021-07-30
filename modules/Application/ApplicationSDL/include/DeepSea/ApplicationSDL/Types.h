/*
 * Copyright 2021 Aaron Barany
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


#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Includes all of the types used in the DeepSea/ApplicationSDL library.
 */

/**
 * @brief Flags for the behavior of the application.
 */
typedef enum dsSDLApplicationFlags
{
	dsSDLApplicationFlags_None = 0,                  ///< No special flags.
	dsSDLApplicationFlags_DisableCompositor = 0x1    ///< Disable the window manager compositor.
} dsSDLApplicationFlags;

#ifdef __cplusplus
}
#endif

// Needs to be after the extern "C" block.
/// @cond
DS_ENUM_BITMASK_OPERATORS(dsSDLApplicationFlags);
/// @endcond
