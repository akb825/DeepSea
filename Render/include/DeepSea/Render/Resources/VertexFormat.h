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
#include <DeepSea/Render/Export.h>
#include <DeepSea/Render/Resources/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for manipulating vertex formats.
 * @see dsVertexFormat
 */

/**
 * @brief Initializes a vertex format.
 * @param format The format to initialize.
 * @return False if format is NULL.
 */
DS_RENDER_EXPORT bool dsVertexFormat_initialize(dsVertexFormat* format);

/**
 * @brief Checks whether or not an attribute is enabled.
 * @param format The vertex format.
 * @param attrib The attribute index.
 * @return True if the attribute is enabled.
 */
DS_RENDER_EXPORT bool dsVertexFormat_getAttribEnabled(const dsVertexFormat* format,
	unsigned int attrib);

/**
 * @brief Sets whether or not an attribute is enabled.
 * @param format The vertex format.
 * @param attrib The attribute index.
 * @param enabled Whether or not the attribute should be enabled.
 * @return False if format is NULL or attrib is out of range..
 */
DS_RENDER_EXPORT bool dsVertexFormat_setAttribEnabled(dsVertexFormat* format, unsigned int attrib,
	bool enabled);

/**
 * @brief Computes the offsets of each vertex attribute and overall size of each vertex.
 * @param format The format to compute the offsets and size for.
 * @return Flase if format is NULL or any format is invalid.
 */
DS_RENDER_EXPORT bool dsVertexFormat_computeOffsetsAndSize(dsVertexFormat* format);

/**
 * @brief Checks if a vertex format is valid for the current target.
 * @param resourceManager The resource manager.
 * @param format The vertex format to check.
 * @return True if the vertex format is valid.
 */
DS_RENDER_EXPORT bool dsVertexFormat_isValid(const dsResourceManager* resourceManager,
	const dsVertexFormat* format);

#ifdef __cplusplus
}
#endif
