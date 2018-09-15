/*
 * Copyright 2018 Aaron Barany
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
#include <DeepSea/Render/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Includes all of the types used in the DeepSea/RenderBootstrap library.
 */

/**
 * @brief Log tag for the RenderBootstrap library.
 */
#define DS_RENDER_BOOTSTRAP_LOG_TAG "render-bootstrap"


/**
 * @brief Enum for the standard renderer types that are supported by DeepSea.
 *
 * The order of the enum is the order in which they are tried. Metal is listed first to prefer the
 * native Metal renderer over using MoltenVK when it's installed. When natively available, Vulkan
 * should be the most efficient renderer. OpenGL is used as a fallback.
 */
typedef enum dsRendererType
{
	dsRendererType_Metal,  ///< Metal renderer, available on Apple systems.
	/** Vulkan renderer. Available on all non-Apple systems with hardware support. */
	dsRendererType_Vulkan,
	dsRendererType_OpenGL, ///< OpenGL or OpenGL ES renderer. Should almost always be available.
	dsRendererType_Default ///< Default renderer type based on system and hardware support.
} dsRendererType;

#ifdef __cplusplus
}
#endif
