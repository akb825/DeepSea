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

#include <DeepSea/Render/Resources/Types.h>

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
 * @brief Gets the size of a material type.
 * @brief type The material type.
 * @return The size of the type.
 */
uint16_t dsMaterialType_size(dsMaterialType type);

/**
 * @brief Gets the machine alignment of a material type.
 *
 * This is used to ensure that the values can be accessed on the CPU without alignment errors.
 *
 * @brief type The material type.
 * @return The machine alignment of the type.
 */
uint16_t dsMaterialType_machineAlignment(dsMaterialType type);

/**
 * @brief Gets the number of rows for a matrix type.
 * @param type The material type.
 * @return The number of matrix rows, or 0 if not a matrix type.
 */
unsigned int dsMaterialType_matrixRows(dsMaterialType type);

/**
 * @brief Gets the number of columns for a matrix type.
 * @param type The material type.
 * @return The number of matrix columns, or 0 if not a matrix type.
 */
unsigned int dsMaterialType_matrixColumns(dsMaterialType type);

/**
 * @brief Gets the type of a matrix column.
 * @param type The material type.
 * @return The type of the column, or dsMaterialType_Count if not a matrix type.
 */
dsMaterialType dsMaterialType_matrixColumnType(dsMaterialType type);

/**
 * @brief Adds the size of an element.
 *
 * This can be used for building up local storage for material elements for access on the CPU. This
 * may not match alignment and packing restrictions for direct access on the GPU.
 *
 * @param[inout] curSize The current size. The element size will be added to this, along with any
 *     adjustment for alignment.
 * @param type The material type.
 * @param count The number of array elements. A value of 0 indicates a single non-array element.
 * @return The offset of the element. This will take the machine alignment into account.
 */
size_t dsMaterialType_addElementSize(size_t* curSize, dsMaterialType type, uint32_t count);

#ifdef __cplusplus
}
#endif
