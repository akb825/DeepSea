/*
 * Copyright 2026 Aaron Barany
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

#include <DeepSea/Core/Memory/Memory.h>
#include <DeepSea/Core/Bits.h>
#include <DeepSea/Core/Error.h>

bool dsAddAlignedSize(size_t* fullSize, size_t newSize, unsigned int alignment)
{
	if (!fullSize || !DS_IS_POWER_OF_2(alignment))
	{
		errno = EINVAL;
		return false;
	}

	size_t alignedFullSize = DS_ALIGNED_SIZE(*fullSize, alignment);
	size_t newAlignedSize = DS_ALIGNED_SIZE(newSize, alignment);
	if (alignedFullSize < *fullSize || newAlignedSize < newSize ||
		!DS_CAN_ADD_SIZES(*fullSize, newSize))
	{
		errno = ERANGE;
		return false;
	}

	*fullSize = alignedFullSize + newAlignedSize;
	return true;
}

bool dsAddAlignedArraySize(size_t* fullSize, size_t elemSize, size_t count, unsigned int alignment)
{
	if (!fullSize || elemSize == 0 || !DS_IS_POWER_OF_2(alignment))
	{
		errno = EINVAL;
		return false;
	}

	size_t alignedFullSize = DS_ALIGNED_SIZE(*fullSize, alignment);
	size_t arraySize = elemSize*count;
	size_t alignedArraySize = DS_ALIGNED_SIZE(arraySize, alignment);
	if (alignedFullSize < *fullSize || alignedArraySize < arraySize ||
		!DS_ARRAY_SIZE_VALID(elemSize, count) || !DS_CAN_ADD_SIZES(*fullSize, arraySize))
	{
		errno = ERANGE;
		return false;
	}

	*fullSize = alignedFullSize + alignedArraySize;
	return true;
}

bool dsAccumulateAlignedSizes(
	size_t* fullSize, const dsMemorySize* sizes, unsigned int sizeCount, unsigned int alignment)
{
	if (!fullSize || (!sizes && sizeCount > 0) || !DS_IS_POWER_OF_2(alignment))
	{
		errno = EINVAL;
		return false;
	}

	size_t alignedFullSize = DS_ALIGNED_SIZE(*fullSize, alignment);
	bool hasOverflow = alignedFullSize < *fullSize;
	bool invalidSize = false;
	for (unsigned int i = 0; i < sizeCount; ++i)
	{
		const dsMemorySize* size = sizes + i;
		size_t arraySize = size->elementSize*size->count;
		size_t alignedArraySize = DS_ALIGNED_SIZE(arraySize, alignment);
		// Assume that if elementSize is 0, a previous call to get a full size failed.
		invalidSize |= (size->elementSize == 0 && size->count != 0);
		hasOverflow |= alignedArraySize < arraySize ||
			(size->elementSize != 0 && !DS_ARRAY_SIZE_VALID(size->elementSize, size->count)) ||
			!DS_CAN_ADD_SIZES(*fullSize, arraySize);
		alignedFullSize += alignedArraySize;
	}

	if (invalidSize)
		return false; // Keep errno the same from the previous failure.
	else if (hasOverflow)
	{
		errno = ERANGE;
		return false;
	}

	*fullSize = alignedFullSize;
	return true;
}
