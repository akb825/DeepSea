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

#if DS_64BIT

inline static uint64_t rotl64(uint64_t x, int8_t r)
{
	return (x << r) | (x >> (64 - r));
}

inline static uint64_t fmix64(uint64_t k)
{
	k ^= k >> 33;
	k *= 0xff51afd7ed558ccdULL;
	k ^= k >> 33;
	k *= 0xc4ceb9fe1a85ec53ULL;
	k ^= k >> 33;

	return k;
}

#endif

static uint32_t hashBytesSmall(uint32_t seed, const void* buffer, size_t size)
{
	// Just the tail portion of dsHashCombineBytes().
	DS_ASSERT(buffer);
	DS_ASSERT(size < sizeof(uint32_t));
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

	h1 ^= (uint32_t)size;

	return fmix32(h1);
}

static uint32_t hashBytes32(uint32_t seed, const void* buffer, size_t size)
{
	// Single iteration of dsHashCombineBytes().
	DS_ASSERT(buffer);
	DS_ASSERT(size == sizeof(uint32_t));
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

	//----------
	// finalization

	h1 ^= (uint32_t)size;

	return fmix32(h1);
}

static uint32_t hashBytes64(uint32_t seed, const void* buffer, size_t size)
{
	// Two iterations of dsHashCombineBytes().
	DS_ASSERT(buffer);
	DS_ASSERT(size == sizeof(uint64_t));
	const uint32_t* block = (const uint32_t*)buffer;
	uint32_t h1 = seed;

	const uint32_t c1 = 0xcc9e2d51;
	const uint32_t c2 = 0x1b873593;

	uint32_t k1 = block[0];

	k1 *= c1;
	k1 = rotl32(k1, 15);
	k1 *= c2;

	h1 ^= k1;
	h1 = rotl32(h1, 13);
	h1 = h1 * 5 + 0xe6546b64;

	k1 = block[1];

	k1 *= c1;
	k1 = rotl32(k1, 15);
	k1 *= c2;

	h1 ^= k1;
	h1 = rotl32(h1, 13);
	h1 = h1 * 5 + 0xe6546b64;

	//----------
	// finalization

	h1 ^= (uint32_t)size;

	return fmix32(h1);
}

uint32_t dsHashBytes(const void* buffer, size_t size)
{
	return dsHashCombineBytes(DEFAULT_SEED, buffer, size);
}

// Optimization on 32-bit ARM with Clang (e.g. Android) causes unaligned access.
#if DS_ARM_32 && DS_CLANG
__attribute__((optnone))
#endif
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

	h1 ^= (uint32_t)size;

	return fmix32(h1);
}

#if DS_64BIT
void dsHashCombineBytes128(void* outResult, const void* seed, const void* buffer,
	size_t size)
{
	// https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp
	DS_ASSERT(outResult);
	DS_ASSERT(seed);
	DS_ASSERT(buffer);
	const uint8_t* data = (const uint8_t*)buffer;
	const size_t nblocks = size/16;

	uint64_t h1 = ((const uint64_t*)seed)[0];
	uint64_t h2 = ((const uint64_t*)seed)[1];

	const uint64_t c1 = 0x87c37b91114253d5LLU;
	const uint64_t c2 = 0x4cf5ad432745937fLLU;

	//----------
	// body

	const uint64_t* blocks = (const uint64_t*)(data);
	for (size_t i = 0; i < nblocks; i++)
	{
		uint64_t k1 = blocks[i*2];
		uint64_t k2 = blocks[i*2 + 1];

		k1 *= c1;
		k1 = rotl64(k1, 31);
		k1 *= c2;
		h1 ^= k1;

		h1 = rotl64(h1, 27);
		h1 += h2;
		h1 = h1*5+0x52dce729;

		k2 *= c2;
		k2 = rotl64(k2, 33);
		k2 *= c1;
		h2 ^= k2;

		h2 = rotl64(h2,31);
		h2 += h1;
		h2 = h2*5+0x38495ab5;
	}

	//----------
	// tail

	const uint8_t* tail = (const uint8_t*)(data + nblocks*16);

	uint64_t k1 = 0;
	uint64_t k2 = 0;

	switch (size & 15)
	{
		case 15: k2 ^= ((uint64_t)tail[14]) << 48;
		case 14: k2 ^= ((uint64_t)tail[13]) << 40;
		case 13: k2 ^= ((uint64_t)tail[12]) << 32;
		case 12: k2 ^= ((uint64_t)tail[11]) << 24;
		case 11: k2 ^= ((uint64_t)tail[10]) << 16;
		case 10: k2 ^= ((uint64_t)tail[ 9]) << 8;
		case  9: k2 ^= ((uint64_t)tail[ 8]) << 0;
		k2 *= c2; k2  = rotl64(k2,33); k2 *= c1; h2 ^= k2;

		case  8: k1 ^= ((uint64_t)tail[ 7]) << 56;
		case  7: k1 ^= ((uint64_t)tail[ 6]) << 48;
		case  6: k1 ^= ((uint64_t)tail[ 5]) << 40;
		case  5: k1 ^= ((uint64_t)tail[ 4]) << 32;
		case  4: k1 ^= ((uint64_t)tail[ 3]) << 24;
		case  3: k1 ^= ((uint64_t)tail[ 2]) << 16;
		case  2: k1 ^= ((uint64_t)tail[ 1]) << 8;
		case  1: k1 ^= ((uint64_t)tail[ 0]) << 0;
		k1 *= c1; k1  = rotl64(k1,31); k1 *= c2; h1 ^= k1;
	};

	//----------
	// finalization

	h1 ^= size;
	h2 ^= size;

	h1 += h2;
	h2 += h1;

	h1 = fmix64(h1);
	h2 = fmix64(h2);

	h1 += h2;
	h2 += h1;

	((uint64_t*)outResult)[0] = h1;
	((uint64_t*)outResult)[1] = h2;
}
#else
void dsHashCombineBytes128(void* outResult, const void* seed, const void* buffer,
	size_t size)
{
	// https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp
	DS_ASSERT(outResult);
	DS_ASSERT(seed);
	DS_ASSERT(buffer);
	const uint8_t* data = (const uint8_t*)buffer;
	const size_t nblocks = size/16;

	uint32_t h1 = ((uint32_t*)seed)[0];
	uint32_t h2 = ((uint32_t*)seed)[1];
	uint32_t h3 = ((uint32_t*)seed)[2];
	uint32_t h4 = ((uint32_t*)seed)[3];

	const uint32_t c1 = 0x239b961b;
	const uint32_t c2 = 0xab0e9789;
	const uint32_t c3 = 0x38b34ae5;
	const uint32_t c4 = 0xa1e38b93;

	//----------
	// body

	const uint32_t* blocks = (const uint32_t*)(data + nblocks*16);
	for (size_t i = -nblocks; i; i++)
	{
		uint32_t k1 = blocks[i*4];
		uint32_t k2 = blocks[i*4 + 1];
		uint32_t k3 = blocks[i*4 + 2];
		uint32_t k4 = blocks[i*4 + 3];

		k1 *= c1;
		k1 = rotl32(k1, 15);
		k1 *= c2;
		h1 ^= k1;

		h1 = rotl32(h1, 19);
		h1 += h2;
		h1 = h1*5+0x561ccd1b;

		k2 *= c2;
		k2 = rotl32(k2, 16);
		k2 *= c3;
		h2 ^= k2;

		h2 = rotl32(h2, 17);
		h2 += h3;
		h2 = h2*5+0x0bcaa747;

		k3 *= c3;
		k3 = rotl32(k3, 17);
		k3 *= c4;
		h3 ^= k3;

		h3 = rotl32(h3, 15);
		h3 += h4;
		h3 = h3*5+0x96cd1c35;

		k4 *= c4;
		k4 = rotl32(k4, 18);
		k4 *= c1;
		h4 ^= k4;

		h4 = rotl32(h4, 13);
		h4 += h1;
		h4 = h4*5+0x32ac3b17;
	}

	//----------
	// tail

	const uint8_t* tail = (const uint8_t*)(data + nblocks*16);

	uint32_t k1 = 0;
	uint32_t k2 = 0;
	uint32_t k3 = 0;
	uint32_t k4 = 0;

	switch (size & 15)
	{
		case 15: k4 ^= tail[14] << 16;
		case 14: k4 ^= tail[13] << 8;
		case 13: k4 ^= tail[12] << 0;
		k4 *= c4; k4  = rotl32(k4,18); k4 *= c1; h4 ^= k4;

		case 12: k3 ^= tail[11] << 24;
		case 11: k3 ^= tail[10] << 16;
		case 10: k3 ^= tail[ 9] << 8;
		case  9: k3 ^= tail[ 8] << 0;
		k3 *= c3; k3  = rotl32(k3,17); k3 *= c4; h3 ^= k3;

		case  8: k2 ^= tail[ 7] << 24;
		case  7: k2 ^= tail[ 6] << 16;
		case  6: k2 ^= tail[ 5] << 8;
		case  5: k2 ^= tail[ 4] << 0;
		k2 *= c2; k2  = rotl32(k2,16); k2 *= c3; h2 ^= k2;

		case  4: k1 ^= tail[ 3] << 24;
		case  3: k1 ^= tail[ 2] << 16;
		case  2: k1 ^= tail[ 1] << 8;
		case  1: k1 ^= tail[ 0] << 0;
		k1 *= c1; k1  = rotl32(k1,15); k1 *= c2; h1 ^= k1;
	};

	//----------
	// finalization

	h1 ^= size;
	h2 ^= size;
	h3 ^= size;
	h4 ^= size;

	h1 += h2; h1 += h3; h1 += h4;
	h2 += h1; h3 += h1; h4 += h1;

	h1 = fmix32(h1);
	h2 = fmix32(h2);
	h3 = fmix32(h3);
	h4 = fmix32(h4);

	h1 += h2; h1 += h3; h1 += h4;
	h2 += h1; h3 += h1; h4 += h1;

	((uint32_t*)outResult)[0] = h1;
	((uint32_t*)outResult)[1] = h2;
	((uint32_t*)outResult)[2] = h3;
	((uint32_t*)outResult)[3] = h4;
}
#endif

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
	return hashBytes64(DEFAULT_SEED, &value, sizeof(uint64_t));
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
