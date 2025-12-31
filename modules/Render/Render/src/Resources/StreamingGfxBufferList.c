/*
 * Copyright 2025 Aaron Barany
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

#include <DeepSea/Render/Resources/StreamingGfxBufferList.h>

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Render/Resources/GfxBuffer.h>

#include <string.h>

uint32_t dsStreamingGfxBufferList_findNext(void* bufferList, uint32_t* bufferCount,
    size_t bufferItemSize, size_t bufferOffset, size_t lastUsedFrameOffset,
    dsDestroyUserDataFunction destroyItemFunc, size_t size, unsigned int frameDelay,
    uint64_t frameNumber)
{
	if (!bufferList || !bufferCount)
		return DS_NO_STREAMING_GFX_BUFFER;

	uint32_t foundBuffer = DS_NO_STREAMING_GFX_BUFFER;
	uint8_t* bufferListBytes = (uint8_t*)bufferList;
	for (uint32_t i = 0; i < *bufferCount;)
	{
		uint8_t* curItemBytes = bufferListBytes + i*bufferItemSize;
		uint64_t* lastUsedFrame = (uint64_t*)(curItemBytes + lastUsedFrameOffset);

		// Skip over all buffers that are still in use, even if too small.
		if (*lastUsedFrame + frameDelay > frameNumber)
		{
			++i;
			continue;
		}

		dsGfxBuffer* curBuffer = *(dsGfxBuffer**)(curItemBytes + bufferOffset);
		DS_ASSERT(curBuffer);
		if (curBuffer->size >= size)
		{
			// Found. Only take the first one, and continue so that smaller buffers can be removed.
			if (foundBuffer == DS_NO_STREAMING_GFX_BUFFER)
			{
				*lastUsedFrame = frameNumber;
				foundBuffer = i;
			}
			++i;
			continue;
		}

		// Wait one additional frame before deleting buffers to handle situations where multiple
		// buffers are needed in a frame.
		if (*lastUsedFrame + frameDelay + 1 > frameNumber)
		{
			++i;
			continue;
		}

		// This buffer is too small. Delete it now since a new one will need to be allocated.
		if (destroyItemFunc)
			destroyItemFunc(curItemBytes);
		else
			dsGfxBuffer_destroy(curBuffer);

		// Constant-time removal since order doesn't matter.
		uint32_t lastBuffer = *bufferCount - 1;
		if (i < lastBuffer)
			memcpy(curItemBytes, bufferListBytes + lastBuffer*bufferItemSize, bufferItemSize);
		--*bufferCount;
	}
	return foundBuffer;
}
