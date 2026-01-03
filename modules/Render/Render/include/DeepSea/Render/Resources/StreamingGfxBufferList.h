/*
 * Copyright 2025-2026 Aaron Barany
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
 * @brief Functions for managing a list of dsGfxBuffer instances for streaming.
 */

/**
 * @brief Default frame delay before re-using a streaming graphics buffer.
 */
#define DS_DEFAULT_STREAMING_GFX_BUFFER_FRAME_DELAY 3

/**
 * @brief Constant for no found streaming graphics buffer.
 */
#define DS_NO_STREAMING_GFX_BUFFER (uint32_t)-1

/**
 * @brief Finds the next buffer to use.
 *
 * Any buffer that is too small and exceeded frameDelay since it was last used will be removed,
 * while the chosen buffer will have its last used frame updated.
 *
 * @param[inout] bufferList The list of buffers as a packed array.
 * @param[inout] bufferCount The number of buffers. This will be updated if small buffers are
 *     removed.
 * @param bufferItemSize The size of each item in bufferList.
 * @param bufferOffset The offset from offsetof() to the graphics buffer member.
 * @param lastUsedFrameOffset The offset from offsetof() to the last used frame as a uint64_t.
 * @param destroyItemFunc The function called to destroy an item. If NULL, the buffer at
 *     bufferOffset will be automatically destroyed.
 * @param minSize The minimum size of the buffer.
 * @param frameDelay The number of frames to wait before re-using a buffer.
 * @param frameNumber The current frame.
 * @return The index of the buffer or DS_NO_STREAMING_GFX_BUFFER if no usable buffer was found.
 */
DS_RENDER_EXPORT uint32_t dsStreamingGfxBufferList_findNext(void* bufferList, uint32_t* bufferCount,
	size_t bufferItemSize, size_t bufferOffset, size_t lastUsedFrameOffset,
	dsDestroyUserDataFunction destroyItemFunc, size_t minSize, unsigned int frameDelay,
	uint64_t frameNumber);

#ifdef __cplusplus
}
#endif
