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

#define DEFAULT_SEED 0xc70f6907U

inline static uint32_t rotl32(uint32_t x, int8_t r)
{
	return (x << r) | (x >> (32 - r));
}

inline static uint32_t fmix32(uint32_t h)
{
	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;

	return h;
}

static uint32_t hashBytesSmall(uint32_t seed, const void* buffer, size_t size)
{
	// Just the tail portion of dsHashCombineBytes().
	DS_ASSERT(buffer);
	const uint8_t* tail = (const uint8_t*)buffer;
	uint32_t k1 = 0;
	uint32_t h1 = seed;

	const uint32_t c1 = 0xcc9e2d51;
	const uint32_t c2 = 0x1b873593;

	switch (size & 3)
	{
		case 3: k1 ^= tail[2] << 16;
		case 2: k1 ^= tail[1] << 8;
		case 1: k1 ^= tail[0];
		k1 *= c1; k1 = rotl32(k1, 15); k1 *= c2; h1 ^= k1;
	};

	//----------
	// finalization

	h1 ^= size;

	return fmix32(h1);
}

static uint32_t hashBytes32(uint32_t seed, const void* buffer, size_t size)
{
	// Single iteration of dsHashCombineBytes().
	DS_ASSERT(buffer);
	uint32_t block = *(const uint32_t*)buffer;
	uint32_t h1 = seed;

	const uint32_t c1 = 0xcc9e2d51;
	const uint32_t c2 = 0x1b873593;

	uint32_t k1 = block;

	k1 *= c1;
	k1 = rotl32(k1, 15);
	k1 *= c2;

	h1 ^= k1;
	h1 = rotl32(h1, 13);
	h1 = h1 * 5 + 0xe6546b64;

	k1 = 0;
	k1 *= c1;
	k1 = rotl32(k1, 15);
	k1 *= c2;
	h1 ^= k1;

	//----------
	// finalization

	h1 ^= size;

	return fmix32(h1);
}

uint32_t dsHashBytes(const void* buffer, size_t size)
{
	return dsHashCombineBytes(DEFAULT_SEED, buffer, size);
}

uint32_t dsHashCombineBytes(uint32_t seed, const void* buffer, size_t size)
{
	// https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp
	DS_ASSERT(buffer);
	const uint8_t* data = (const uint8_t*)buffer;
	const size_t nblocks = size/4;

	uint32_t h1 = seed;

	const uint32_t c1 = 0xcc9e2d51;
	const uint32_t c2 = 0x1b873593;

	//----------
	// body
	const uint32_t* blocks = (const uint32_t*)data;
	for (size_t i = 0; i < nblocks; ++i)
	{
		uint32_t k1 = blocks[i];

		k1 *= c1;
		k1 = rotl32(k1, 15);
		k1 *= c2;

		h1 ^= k1;
		h1 = rotl32(h1, 13);
		h1 = h1 * 5 + 0xe6546b64;
	}

	//----------
	// tail

	const uint8_t* tail = (const uint8_t*)(data + nblocks*4);
	uint32_t k1 = 0;

	switch (size & 3)
	{
		case 3: k1 ^= tail[2] << 16;
		case 2: k1 ^= tail[1] << 8;
		case 1: k1 ^= tail[0];
		k1 *= c1; k1 = rotl32(k1, 15); k1 *= c2; h1 ^= k1;
	};

	//----------
	// finalization

	h1 ^= size;

	return fmix32(h1);
}

uint32_t dsHashCombine(uint32_t first, uint32_t second)
{
	// Uses the same algorithm as boost::hash_combine()
	return first ^ (second + 0x9e3779b9 + (first << 6) + (first >> 2));
}

uint32_t dsHashString(const void* string)
{
	if (!string)
		return DEFAULT_SEED;

	return dsHashBytes(string, strlen((const char*)string));
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
	uint8_t value = ptr ? *(const uint8_t*)ptr : 0;
	return hashBytesSmall(DEFAULT_SEED, &value, sizeof(uint8_t));
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
	uint16_t value = ptr ? *(const uint16_t*)ptr : 0;
	return hashBytesSmall(DEFAULT_SEED, &value, sizeof(uint16_t));
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
	uint32_t value = ptr ? *(const uint32_t*)ptr : 0;
	return hashBytes32(DEFAULT_SEED, &value, sizeof(uint32_t));
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
	uint64_t value = ptr ? *(const uint64_t*)ptr : 0;
	return dsHashBytes(&value, sizeof(uint64_t));
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

uint32_t dsHashFloat(const void* ptr)
{
	float value = ptr ? *(const float*)ptr : 0;
	// Handle -0
	value = value != 0.0f ? value : 0.0f;
	return hashBytes32(DEFAULT_SEED, &value, sizeof(uint32_t));
}

bool dsHashFloatEqual(const void* first, const void* second)
{
	if (first == second)
		return true;
	else if ((first && !second) || (!first && second))
		return false;

	return *(const float*)first == *(const float*)second;
}

uint32_t dsHashDouble(const void* ptr)
{
	double value = ptr ? *(const double*)ptr : 0;
	// Handle -0
	value = value != 0.0 ? value : 0.0;
	return dsHashBytes(&value, sizeof(double));
}

bool dsHashDoubleEqual(const void* first, const void* second)
{
	if (first == second)
		return true;
	else if ((first && !second) || (!first && second))
		return false;

	return *(const double*)first == *(const double*)second;
}
