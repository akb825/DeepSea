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

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Assert.h>
#include <string.h>

static uint32_t hashInteger(uint32_t value)
{
	// http://stackoverflow.com/questions/664014/what-integer-hash-function-are-good-that-accepts-an-integer-hash-key
	value = ((value >> 16) ^ value) * 0x45d9f3b;
    value = ((value >> 16) ^ value) * 0x45d9f3b;
    value = (value >> 16) ^ value;
    return value;
}

uint32_t dsHashCombine(uint32_t first, uint32_t second)
{
	// Uses the same algorithm as boost::hash_combine()
	return first ^ (second + 0x9e3779b9 + (first << 6) + (first >> 2));
}

uint32_t dsHashString(const void* string)
{
	// djb2 algorithm
	uint32_t hash = 5381;
	if (!string)
		return hash;

	for (const char* c = (const char*)string; *c; ++c)
		hash = ((hash << 5) + hash) + *c; /* hash * 33 + c */

	return hash;
}

bool dsHashStringEqual(const void* first, const void* second)
{
	if (first == second)
		return true;
	else if ((first && !second) || (!first && second))
		return false;

	return strcmp((const char*)first, (const char*)second) == 0;
}

uint32_t dsHash8(const void* ptr)
{
	return hashInteger(ptr ? *(const uint8_t*)ptr : 0);
}

bool dsHash8Equal(const void* first, const void* second)
{
	if (first == second)
		return true;
	else if ((first && !second) || (!first && second))
		return false;

	return *(const uint8_t*)first == *(const uint8_t*)second;
}

uint32_t dsHash16(const void* ptr)
{
	return hashInteger(ptr ? *(const uint16_t*)ptr : 0);
}

bool dsHash16Equal(const void* first, const void* second)
{
	if (first == second)
		return true;
	else if ((first && !second) || (!first && second))
		return false;

	return *(const uint16_t*)first == *(const uint16_t*)second;
}

uint32_t dsHash32(const void* ptr)
{
	return hashInteger(ptr ? *(const uint32_t*)ptr : 0);
}

bool dsHash32Equal(const void* first, const void* second)
{
	if (first == second)
		return true;
	else if ((first && !second) || (!first && second))
		return false;

	return *(const uint32_t*)first == *(const uint32_t*)second;
}

uint32_t dsHash64(const void* ptr)
{
	if (!ptr)
		return hashInteger(0);

	const uint32_t* integers = (const uint32_t*)ptr;
	return dsHashCombine(hashInteger(integers[0]), hashInteger(integers[1]));
}

bool dsHash64Equal(const void* first, const void* second)
{
	if (first == second)
		return true;
	else if ((first && !second) || (!first && second))
		return false;

	return *(const uint64_t*)first == *(const uint64_t*)second;
}

uint32_t dsHashSizeT(const void* ptr)
{
#if DS_64BIT
	DS_STATIC_ASSERT(sizeof(size_t) == sizeof(uint64_t), unexpected_sizeof_size_t);
	return dsHash64(ptr);
#else
	DS_STATIC_ASSERT(sizeof(size_t) == sizeof(uint32_t), unexpected_sizeof_size_t);
	return dsHash32(ptr);
#endif
}

bool dsHashSizeTEqual(const void* first, const void* second)
{
	if (first == second)
		return true;
	else if ((first && !second) || (!first && second))
		return false;

	return *(const uint32_t*)first == *(const uint32_t*)second;
}

uint32_t dsHashPointer(const void* ptr)
{
#if DS_64BIT
	DS_STATIC_ASSERT(sizeof(void*) == sizeof(uint64_t), unexpected_sizeof_void_ptr);
	return dsHash64(&ptr);
#else
	DS_STATIC_ASSERT(sizeof(void*) == sizeof(uint32_t), unexpected_sizeof_void_ptr);
	return dsHash32(&ptr);
#endif
}

bool dsHashPointerEqual(const void* first, const void* second)
{
	return first == second;
}
