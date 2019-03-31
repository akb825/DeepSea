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

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief File containing the renderer IDs for Metal.
 */

/**
 * @brief Log tag for the RenderMetal library.
 */
#define DS_RENDER_METAL_LOG_TAG "metal"

/**
 * @brief Constant for the renderer type ID of the Metal renderer.
 */
#define DS_MTL_RENDERER_ID DS_FOURCC('M', 'T', 'L', 0)

#ifdef __cplusplus
}
#endif
