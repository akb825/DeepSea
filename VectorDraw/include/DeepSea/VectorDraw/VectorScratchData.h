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
#include <DeepSea/VectorDraw/Export.h>
#include <DeepSea/VectorDraw/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating scratch data for vector images.
 *
 * A scratch data instance is required for creating vector images, and gives better control over
 * the memory that's allocated that managing it completely internally. The same dsVectorScratchData
 * instance may be re-used across multiple vector image creations, so long as they aren't performed
 * simultaneously on different threads, to help minimize the number of allocations. It will use an
 * exponential allocation scheme similar to std::vector in C++, and should eventually reach a stable
 * state where no more allocations will be used.
 *
 * @see dsVectorScatchData
 */

/**
 * @brief Creates scratch vector data.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the scratch data with. This must support freeing data.
 * @return The created data, or NULL if it couldn't be created.
 */
DS_VECTORDRAW_EXPORT dsVectorScratchData* dsVectorScratchData_create(dsAllocator* allocator);

/**
 * @brief Destroys a scratch data instance.
 * @param data The data to destroy.
 */
DS_VECTORDRAW_EXPORT void dsVectorScratchData_destroy(dsVectorScratchData* data);

#ifdef __cplusplus
}
#endif
