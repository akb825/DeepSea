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

#include <DeepSea/Render/Resources/GfxBuffer.h>

#include <DeepSea/Core/Log.h>
#include <DeepSea/Render/Resources/ResourceManager.h>
#include <DeepSea/Render/Types.h>
#include <errno.h>

dsGfxBuffer* dsGfxBuffer_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	int usage, int memoryHints, size_t size, const void* data)
{
	if (!resourceManager || (!allocator && !resourceManager->allocator) ||
		!resourceManager->createBufferFunc || !resourceManager->destroyBufferFunc)
	{
		errno = EINVAL;
		return NULL;
	}

	if (usage == 0)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"At least one usage flag must be set when creating a buffer.");
		return NULL;
	}

	if (memoryHints == 0)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"At least one memory hint flag must be set when creating a buffer.");
		return NULL;
	}

	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Resources can only be manipulated from the main thread or "
			"threads that have created a resource context.");
		return NULL;
	}

	return resourceManager->createBufferFunc(resourceManager, allocator, usage, memoryHints, size,
		data);
}

void* dsGfxBuffer_map(dsGfxBuffer* buffer, int flags, size_t offset, size_t size)
{
	if (!buffer || !buffer->resourceManager || !buffer->resourceManager->mapBufferFunc ||
		!buffer->resourceManager->unmapBufferFunc)
	{
		errno = EINVAL;
		return NULL;
	}

	dsResourceManager* resourceManager = buffer->resourceManager;
	dsGfxBufferMapSupport support = resourceManager->bufferMapSupport;
	if (support == dsGfxBufferMapSupport_None)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Buffer mapping not supported on the current device.");
		return NULL;
	}
	else if ((flags & dsGfxBufferMap_Persistent) && support != dsGfxBufferMapSupport_Persistent)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Persistent buffer mapping not supported on the current device.");
		return NULL;
	}

	if ((size == DS_MAP_FULL_BUFFER && offset > size) ||
		(size != DS_MAP_FULL_BUFFER && offset + size > buffer->size))
	{
		errno = ERANGE;
		return NULL;
	}

	if (flags == 0)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"At least one buffer map flag must set when mapping a buffer.");
		return NULL;
	}

	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Resources can only be manipulated from the main thread or "
			"threads that have created a resource context.");
		return NULL;
	}

	if (support == dsGfxBufferMapSupport_Full)
	{
		void* mappedMem = resourceManager->mapBufferFunc(resourceManager, buffer, flags, 0,
			DS_MAP_FULL_BUFFER);
		return ((uint8_t*)mappedMem + offset);
	}
	else
		return buffer->resourceManager->mapBufferFunc(resourceManager, buffer, flags, offset, size);
}

bool dsGfxBuffer_unmap(dsGfxBuffer* buffer)
{
	if (!buffer || !buffer->resourceManager || !buffer->resourceManager->unmapBufferFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsResourceManager* resourceManager = buffer->resourceManager;
	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Resources can only be manipulated from the main thread or "
			"threads that have created a resource context.");
		return false;
	}

	return resourceManager->unmapBufferFunc(resourceManager, buffer);
}

bool dsGfxBuffer_flush(dsGfxBuffer* buffer, size_t offset, size_t size)
{
	if (!buffer || !buffer->resourceManager || !buffer->resourceManager->flushBufferFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsResourceManager* resourceManager = buffer->resourceManager;
	if (resourceManager->bufferMapSupport != dsGfxBufferMapSupport_Persistent)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Persistent buffer mapping not supported on the current device.");
		return false;
	}

	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Resources can only be manipulated from the main thread or "
			"threads that have created a resource context.");
		return false;
	}

	return resourceManager->flushBufferFunc(resourceManager, buffer, offset, size);
}

bool dsGfxBuffer_invalidate(dsGfxBuffer* buffer, size_t offset, size_t size)
{
	if (!buffer || !buffer->resourceManager || !buffer->resourceManager->invalidateBufferFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsResourceManager* resourceManager = buffer->resourceManager;
	if (resourceManager->bufferMapSupport != dsGfxBufferMapSupport_Persistent)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Persistent buffer mapping not supported on the current device.");
		return false;
	}

	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Resources can only be manipulated from the main thread or "
			"threads that have created a resource context.");
		return false;
	}

	return resourceManager->invalidateBufferFunc(resourceManager, buffer, offset, size);
}

bool dsGfxBuffer_copyData(dsGfxBuffer* buffer, size_t offset, size_t size, const void* data)
{
	if (!buffer || !buffer->resourceManager || !buffer->resourceManager->copyBufferDataFunc ||
		!data)
	{
		errno = EINVAL;
		return false;
	}

	if (offset + size > buffer->size)
	{
		errno = ERANGE;
		return false;
	}

	dsResourceManager* resourceManager = buffer->resourceManager;
	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Resources can only be manipulated from the main thread or "
			"threads that have created a resource context.");
		return false;
	}

	return resourceManager->copyBufferDataFunc(resourceManager, buffer, offset, size, data);
}

bool dsGfxBuffer_copy(dsGfxBuffer* srcBuffer, size_t srcOffset, dsGfxBuffer* dstBuffer,
	size_t dstOffset, size_t size)
{
	if (!srcBuffer || !srcBuffer->resourceManager || !srcBuffer->resourceManager->copyBufferFunc ||
		!dstBuffer || dstBuffer->resourceManager != srcBuffer->resourceManager)
	{
		errno = EINVAL;
		return false;
	}

	if (srcOffset + size > srcBuffer->size || dstOffset + size > dstBuffer->size)
	{
		errno = ERANGE;
		return false;
	}

	dsResourceManager* resourceManager = srcBuffer->resourceManager;
	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Resources can only be manipulated from the main thread or "
			"threads that have created a resource context.");
		return false;
	}

	return resourceManager->copyBufferFunc(resourceManager, srcBuffer, srcOffset, dstBuffer,
		dstOffset, size);
}

bool dsGfxBuffer_destroy(dsGfxBuffer* buffer)
{
	if (!buffer || !buffer->resourceManager || !buffer->resourceManager->destroyBufferFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsResourceManager* resourceManager = buffer->resourceManager;
	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Resources can only be manipulated from the main thread or "
			"threads that have created a resource context.");
		return false;
	}

	return resourceManager->destroyBufferFunc(resourceManager, buffer);
}
