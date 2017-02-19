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
#include <DeepSea/Render/Resources/Types.h>
#include <DeepSea/Render/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for dealing with graphics buffers.
 *
 * All functions must either be called on the main thread or on a thread with an active resource
 * context. A resource shouldn't be accessed simultaneously across multiple threads.
 *
 * @see dsGfxFormat
 */

/**
 * @brief Creates a graphics buffer.
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager to create the buffer from.
 * @param allocator The allocator to create the graphics buffer with. If NULL, it will use the same
 *     allocator as the resource manager.
 * @param usage How the buffer will be used. This should be a combination of dsGfxBufferUsage flags.
 * @param memoryHints Hints for how the memory for the buffer will be used. This should be a
 *     combination of dsGfxMemory flags.
 * @param data The initial data for the buffer, or NULL to leave uninitialized. This must be the
 *     same size as the buffer.
 * @param size The size of the buffer. This must be given even if data is NULL.
 * @return The created buffer, or NULL if it couldn't be created. errno will be set to an
 *     appropriate value on failure.
 */
DS_RENDER_EXPORT dsGfxBuffer* dsGfxBuffer_create(dsResourceManager* resourceManager,
	dsAllocator* allocator, int usage, int memoryHints, const void* data, size_t size);

/**
 * @brief Maps a range of a graphics buffer to memory.
 * @remark errno will be set on failure.
 * @param buffer The buffer to map.
 * @param flags The flags describing how to map the memory. This should be a combination of
 *     dsGfxBufferMap flags
 * @param offset The offset into the buffer to map. This must be aligned with minMappingAlignment
 *     from dsResourceManager.
 * @param size The number of bytes to map. This may be set to DS_MAP_FULL_BUFFER to map from the
 *     offset to the end of the buffer.
 * @return A pointer to the mapped memory or NULL if the memory couldn't be mapped. errno will be
 *     set to an appropriate value on failure.
 */
DS_RENDER_EXPORT void* dsGfxBuffer_map(dsGfxBuffer* buffer, int flags, size_t offset, size_t size);

/**
 * @brief Unmaps previously mapped memory from a graphics buffer.
 * @remark errno will be set on failure.
 * @param buffer The buffer to unmap.
 * @return False if the memory couldn't be unmapped. errno will be set to an appropriate value on
 *     failure.
 */
DS_RENDER_EXPORT bool dsGfxBuffer_unmap(dsGfxBuffer* buffer);

/**
 * @brief Flushes writes to a mapped memory range for a buffer.
 *
 * This is generally used for persistently mapped memory for a non-coherent buffer. This guarantees
 * writes from the CPU will be visible from the GPU.
 *
 * @remark errno will be set on failure.
 * @param buffer The buffer to flush.
 * @param offset The offset of the range to flush.
 * @param size The size of the memory to flush.
 * @return False if the memory couldn't be flushed. errno will be set to an appropriate value on
 *     failure.
 */
DS_RENDER_EXPORT bool dsGfxBuffer_flush(dsGfxBuffer* buffer, size_t offset, size_t size);

/**
 * @brief Invlidates reads to a mapped memory range for a buffer.
 *
 * This is generally used for persistently mapped memory for a non-coherent buffer. This guarantees
 * writes from the GPU will be visible from the CPU.
 *
 * @remark errno will be set on failure.
 * @param buffer The buffer to invalidate.
 * @param offset The offset of the range to invalidate.
 * @param size The size of the memory to invalidate.
 * @return False if the memory couldn't be invalidated. errno will be set to an appropriate value on
 *     failure.
 */
DS_RENDER_EXPORT bool dsGfxBuffer_invalidate(dsGfxBuffer* buffer, size_t offset, size_t size);

/**
 * @brief Copies data to a buffer on the command queue.
 *
 * This queues the copy on a command buffer, so the thread that processes this doesn't need a
 * resource context.
 *
 * @remark errno will be set on failure.
 * @param commandBuffer The command buffer to process the copy on.
 * @param buffer The buffer to copy the data to. This must have been created with
 *     dsGfxBufferUsage_CopyTo.
 * @param offset The offset into the buffer.
 * @param data The data to copy to the buffer.
 * @param size The size of the data to copy.
 * @return False if the data couldn't be copied. errno will be set to an appropriate value on
 *     failure.
 */
DS_RENDER_EXPORT bool dsGfxBuffer_copyData(dsCommandBuffer* commandBuffer, dsGfxBuffer* buffer,
	size_t offset, const void* data, size_t size);

/**
 * @brief Copies data from one buffer to another.
 *
 * This queues the copy on a command buffer, so the thread that processes this doesn't need a
 * resource context.
 *
 * @remark errno will be set on failure.
 * @param commandBuffer The command buffer to process the copy on.
 * @param srcBuffer The buffer to copy the data from. This must have been created with
 *     dsGfxBufferUsage_CopyFrom.
 * @param srcOffset The offset into the source buffer.
 * @param dstBuffer The buffer to copy to. This must have been created with dsGfxBufferUsage_CopyTo.
 * @param dstOffset The offset into the destination buffer.
 * @param size The size of the data to copy.
 * @return False if the data couldn't be copied. errno will be set to an appropriate value on
 *     failure.
 */
DS_RENDER_EXPORT bool dsGfxBuffer_copy(dsCommandBuffer* commandBuffer, dsGfxBuffer* srcBuffer,
	size_t srcOffset, dsGfxBuffer* dstBuffer, size_t dstOffset, size_t size);

/**
 * @brief Destroys a graphics buffer.
 * @remark errno will be set on failure.
 * @param buffer The buffer to destroy.
 * @return False if the buffer couldn't be destroyed. errno will be set to an appropriate value on
 *     failure.
 */
DS_RENDER_EXPORT bool dsGfxBuffer_destroy(dsGfxBuffer* buffer);

#ifdef __cplusplus
}
#endif
