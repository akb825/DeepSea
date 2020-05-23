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

#include <DeepSea/Core/Containers/StringPool.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <string.h>

bool dsStringPool_initialize(dsStringPool* stringPool)
{
	if (!stringPool)
	{
		errno = EINVAL;
		return false;
	}

	memset(stringPool, 0, sizeof(*stringPool));
	return true;
}

bool dsStringPool_reserve(dsStringPool* stringPool, const char* string)
{
	if (!stringPool || !string)
	{
		errno = EINVAL;
		return false;
	}

	if (stringPool->strings)
	{
		errno = EPERM;
		return false;
	}

	stringPool->reservedSize += strlen(string) + 1;
	return true;
}

bool dsStringPool_allocate(dsStringPool* stringPool, dsAllocator* allocator)
{
	if (!stringPool || !allocator)
	{
		errno = EINVAL;
		return false;
	}

	if (stringPool->strings)
	{
		errno = EPERM;
		return false;
	}

	if (stringPool->reservedSize == 0)
		return true;

	stringPool->strings = (char*)dsAllocator_alloc(allocator, stringPool->reservedSize);
	if (!stringPool->strings)
		return false;

	stringPool->allocator = dsAllocator_keepPointer(allocator);
	stringPool->size = 0;
	return true;
}

const char* dsStringPool_insert(dsStringPool* stringPool, const char* string)
{
	if (!stringPool || !string)
	{
		errno = EINVAL;
		return false;
	}

	if (!stringPool->strings)
	{
		errno = EPERM;
		return false;
	}

	size_t size = strlen(string) + 1;
	if (!DS_IS_BUFFER_RANGE_VALID(stringPool->size, size, stringPool->reservedSize))
	{
		errno = ENOMEM;
		return false;
	}

	char* insertedString = stringPool->strings + stringPool->size;
	strcpy(insertedString, string);
	stringPool->size += size;
	return insertedString;
}

void dsStringPool_shutdown(dsStringPool* stringPool)
{
	if (!stringPool)
		return;

	if (stringPool->allocator)
		DS_VERIFY(dsAllocator_free(stringPool->allocator, stringPool->strings));
	memset(stringPool, 0, sizeof(*stringPool));
}
