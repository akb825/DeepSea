/*
 * Copyright 2017 Aaron Barany
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
#include <DeepSea/Render/Resources/Types.h>
#include <DeepSea/Render/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Helper functions for manipulating a material type.
 * @see dsMaterialType
 */

/**
 * @brief Constant for an invalid offset.
 */
#define DS_INVALID_MATERIAL_OFFSET (size_t)-1

/**
 * @brief Gets the number of rows for a matrix type.
 * @param type The material type.
 * @return The number of matrix rows, or 0 if not a matrix type.
 */
DS_RENDER_EXPORT unsigned int dsMaterialType_matrixRows(dsMaterialType type);

/**
 * @brief Gets the number of columns for a matrix type.
 * @param type The material type.
 * @return The number of matrix columns, or 0 if not a matrix type.
 */
DS_RENDER_EXPORT unsigned int dsMaterialType_matrixColumns(dsMaterialType type);

/**
 * @brief Gets the type of a matrix column.
 * @param type The material type.
 * @return The type of the column, or dsMaterialType_Count if not a matrix type.
 */
DS_RENDER_EXPORT dsMaterialType dsMaterialType_matrixColumnType(dsMaterialType type);

/**
 * @brief Gets the type of a matrix row.
 * @param type The material type.
 * @return The type of the row, or dsMaterialType_Count if not a matrix type.
 */
DS_RENDER_EXPORT dsMaterialType dsMaterialType_matrixColumnType(dsMaterialType type);

/**
 * @brief Gets the size of a material type on the CPU.
 * @param type The material type.
 * @return The size of the type.
 */
DS_RENDER_EXPORT uint16_t dsMaterialType_cpuSize(dsMaterialType type);

/**
 * @brief Gets the alignment of a material type on the CPU.
 *
 * This is used to ensure that the values can be accessed on the CPU without alignment errors.
 *
 * @param type The material type.
 * @return The alignment of the type.
 */
DS_RENDER_EXPORT uint16_t dsMaterialType_cpuAlignment(dsMaterialType type);

/**
 * @brief Adds the size of an element for access on the CPU.
 *
 * This can be used for building up local storage for material elements for access on the CPU. This
 * may not match alignment and packing restrictions for direct access on the GPU.
 *
 * @param[inout] curSize The current size. The element size will be added to this, along with any
 *     adjustment for alignment.
 * @param type The material type.
 * @param count The number of array elements. A value of 0 indicates a single non-array element.
 * @return The offset of the element. This will take the CPU alignment into account. If not a valid
 *     type, DS_INVALID_MATERIAL_OFFSET will be returned.
 */
DS_RENDER_EXPORT size_t dsMaterialType_addElementCpuSize(size_t* curSize, dsMaterialType type,
	uint32_t count);

/**
 * @brief Gets the size of a material type for a uniform block on the GPU.
 *
 * This follows the std140 layout standard.
 *
 * @param type The material type.
 * @param isArray True if the element is an array.
 * @return The size of the type.
 */
DS_RENDER_EXPORT uint16_t dsMaterialType_blockSize(dsMaterialType type, bool isArray);

/**
 * @brief Gets the alignment of a material type for a uniform block on the GPU.
 *
 * This follows the std140 layout standard.
 *
 * @param type The material type.
 * @param isArray True if the element is an array.
 * @return The alignment of the type.
 */
DS_RENDER_EXPORT uint16_t dsMaterialType_blockAlignment(dsMaterialType type, bool isArray);

/**
 * @brief Adds the size of an element for access by a uniform block on the GPU.
 *
 * This can be used for setting up storage and accessing material elements used with a uniform block
 * used by the GPU. This follows the std140 layout standard.
 *
 * @param[inout] curSize The current size. The element size will be added to this, along with any
 *     adjustment for alignment.
 * @param type The material type.
 * @param count The number of array elements. A value of 0 indicates a single non-array element.
 * @return The offset of the element. This will take the uniform block alignment into account. If
 *    not a valid type, DS_INVALID_MATERIAL_OFFSET will be returned.
 */
DS_RENDER_EXPORT size_t dsMaterialType_addElementBlockSize(size_t* curSize, dsMaterialType type,
	uint32_t count);

/**
 * @brief Gets the size of a material type for a storage buffer on the GPU.
 *
 * This follows the std430 layout standard.
 *
 * @param type The material type.
 * @return The size of the type.
 */
DS_RENDER_EXPORT uint16_t dsMaterialType_bufferSize(dsMaterialType type);

/**
 * @brief Gets the alignment of a material type for a storage buffer on the GPU.
 *
 * This follows the std430 layout standard.
 *
 * @param type The material type.
 * @return The alignment of the type.
 */
DS_RENDER_EXPORT uint16_t dsMaterialType_bufferAlignment(dsMaterialType type);

/**
 * @brief Adds the size of an element for access by a storage buffer on the GPU.
 *
 * This can be used for setting up storage and accessing material elements used with a storage
 * buffer used by the GPU. This follows the std430 layout standard.
 *
 * @param[inout] curSize The current size. The element size will be added to this, along with any
 *     adjustment for alignment.
 * @param type The material type.
 * @param count The number of array elements. A value of 0 indicates a single non-array element.
 * @return The offset of the element. This will take the storage buffer alignment into account. If
 *    not a valid type, DS_INVALID_MATERIAL_OFFSET will be returned.
 */
DS_RENDER_EXPORT size_t dsMaterialType_addElementBufferSize(size_t* curSize, dsMaterialType type,
	uint32_t count);

#ifdef __cplusplus
}
#endif
