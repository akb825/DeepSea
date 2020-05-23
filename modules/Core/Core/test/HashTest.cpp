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
#include <gtest/gtest.h>

TEST(HashTest, HashCombineBytes)
{
	// Same test as run in reference murmur implementation
	// https://github.com/aappleby/smhasher/blob/master/src/main.cpp
	// https://github.com/aappleby/smhasher/blob/master/src/KeysetTest.cpp
	// This ensures that nothing broke in transferring the implementation.
	uint8_t key[256];
	uint32_t hashes[256];
	for (unsigned int i = 0; i < 256; ++i)
	{
		key[i] = (uint8_t)i;
		hashes[i] = dsHashCombineBytes(256 - i, key, i);
	}

	uint32_t finalHash = dsHashCombineBytes(0, hashes, sizeof(hashes));
	EXPECT_EQ(0xB0F57EE3, finalHash);
}

TEST(HashTest, HashCombineBytes128)
{
	// Same test as run in reference murmur implementation
	// https://github.com/aappleby/smhasher/blob/master/src/main.cpp
	// https://github.com/aappleby/smhasher/blob/master/src/KeysetTest.cpp
	// This ensures that nothing broke in transferring the implementation.
	uint8_t key[256];
	uint32_t hashes[256][4];
	for (unsigned int i = 0; i < 256; ++i)
	{
		key[i] = (uint8_t)i;
#if DS_64BIT
		uint64_t seed[2] = {256 - i, 256 - i};
#else
		uint32_t seed[4] = {256 - i, 256 - i, 256 - i, 256 - i};
#endif
		dsHashCombineBytes128(hashes[i], seed, key, i);
	}

	uint32_t zeroSeed[4] = {0, 0, 0, 0};
	uint8_t finalHash[16];
	dsHashCombineBytes128(finalHash, zeroSeed, hashes, sizeof(hashes));

	uint32_t testResult = finalHash[0] | (finalHash[1] << 8) | (finalHash[2] << 16) |
		(finalHash[3] << 24);
#if DS_64BIT
	EXPECT_EQ(0x6384BA69U, testResult);
#else
	EXPECT_EQ(0xB3ECE62AU, testResult);
#endif
}

TEST(HashTest, HashCombine)
{
	EXPECT_NE(dsHashCombine(1, 2), dsHashCombine(2, 1));
}

TEST(HashTest, HashString)
{
	std::string str1 = "test1";
	std::string str2 = "test2";

	EXPECT_EQ(dsHashString("test1"), dsHashString(str1.c_str()));
	EXPECT_EQ(dsHashString("test2"), dsHashString(str2.c_str()));
	EXPECT_NE(dsHashString(str1.c_str()), dsHashString(str2.c_str()));
	EXPECT_EQ(dsHashString("test1"), dsHashBytes(str1.c_str(), str1.length()));
	EXPECT_EQ(dsHashCombineString(1234, "test1"), dsHashCombineBytes(1234, str1.c_str(),
		str1.length()));

	EXPECT_TRUE(dsHashStringEqual("test1", str1.c_str()));
	EXPECT_TRUE(dsHashStringEqual("test2", str2.c_str()));
	EXPECT_FALSE(dsHashStringEqual(str1.c_str(), str2.c_str()));

	EXPECT_TRUE(dsHashStringEqual(nullptr, nullptr));
	EXPECT_FALSE(dsHashStringEqual(str1.c_str(), nullptr));
	EXPECT_FALSE(dsHashStringEqual(nullptr, str2.c_str()));
	EXPECT_TRUE(dsHashStringEqual(str1.c_str(), str1.c_str()));
}

TEST(HashTest, Hash8)
{
	uint8_t val1 = 123;
	uint8_t val2 = 45;
	EXPECT_NE(dsHash8(&val1), dsHash8(&val2));
	EXPECT_EQ(dsHashBytes(&val1, sizeof(val1)), dsHash8(&val1));
	EXPECT_EQ(dsHashCombineBytes(1234, &val1, sizeof(val1)), dsHashCombine8(1234, &val1));

	EXPECT_FALSE(dsHash8Equal(&val1, &val2));
	val2 = val1;
	EXPECT_TRUE(dsHash8Equal(&val1, &val2));

	EXPECT_TRUE(dsHash8Equal(nullptr, nullptr));
	EXPECT_FALSE(dsHash8Equal(&val1, nullptr));
	EXPECT_FALSE(dsHash8Equal(nullptr, &val2));
	EXPECT_TRUE(dsHash8Equal(&val1, &val2));
}

TEST(HashTest, Hash16)
{
	uint16_t val1 = 123;
	uint16_t val2 = 456;
	EXPECT_NE(dsHash16(&val1), dsHash16(&val2));
	EXPECT_EQ(dsHashBytes(&val1, sizeof(val1)), dsHash16(&val1));
	EXPECT_EQ(dsHashCombineBytes(1234, &val1, sizeof(val1)), dsHashCombine16(1234, &val1));

	EXPECT_FALSE(dsHash16Equal(&val1, &val2));
	val2 = val1;
	EXPECT_TRUE(dsHash16Equal(&val1, &val2));

	EXPECT_TRUE(dsHash16Equal(nullptr, nullptr));
	EXPECT_FALSE(dsHash16Equal(&val1, nullptr));
	EXPECT_FALSE(dsHash16Equal(nullptr, &val2));
	EXPECT_TRUE(dsHash16Equal(&val1, &val2));
}

TEST(HashTest, Hash32)
{
	uint32_t val1 = 123;
	uint32_t val2 = 456;
	EXPECT_NE(dsHash32(&val1), dsHash32(&val2));
	EXPECT_EQ(dsHashBytes(&val1, sizeof(val1)), dsHash32(&val1));
	EXPECT_EQ(dsHashCombineBytes(1234, &val1, sizeof(val1)), dsHashCombine32(1234, &val1));

	EXPECT_FALSE(dsHash32Equal(&val1, &val2));
	val2 = val1;
	EXPECT_TRUE(dsHash32Equal(&val1, &val2));

	EXPECT_TRUE(dsHash32Equal(nullptr, nullptr));
	EXPECT_FALSE(dsHash32Equal(&val1, nullptr));
	EXPECT_FALSE(dsHash32Equal(nullptr, &val2));
	EXPECT_TRUE(dsHash32Equal(&val1, &val2));
}

TEST(HashTest, Hash64)
{
	uint64_t val1 = 123;
	uint64_t val2 = 456;
	EXPECT_NE(dsHash64(&val1), dsHash64(&val2));
	EXPECT_EQ(dsHashBytes(&val1, sizeof(val1)), dsHash64(&val1));
	EXPECT_EQ(dsHashCombineBytes(1234, &val1, sizeof(val1)), dsHashCombine64(1234, &val1));

	EXPECT_FALSE(dsHash64Equal(&val1, &val2));
	val2 = val1;
	EXPECT_TRUE(dsHash64Equal(&val1, &val2));

	EXPECT_TRUE(dsHash64Equal(nullptr, nullptr));
	EXPECT_FALSE(dsHash64Equal(&val1, nullptr));
	EXPECT_FALSE(dsHash64Equal(nullptr, &val2));
	EXPECT_TRUE(dsHash64Equal(&val1, &val2));
}

TEST(HashTest, HashSizeT)
{
	size_t val1 = 123;
	size_t val2 = 456;
	EXPECT_NE(dsHashSizeT(&val1), dsHashSizeT(&val2));
	EXPECT_EQ(dsHashBytes(&val1, sizeof(val1)), dsHashSizeT(&val1));
	EXPECT_EQ(dsHashCombineBytes(1234, &val1, sizeof(val1)), dsHashCombineSizeT(1234, &val1));

	EXPECT_FALSE(dsHashSizeTEqual(&val1, &val2));
	val2 = val1;
	EXPECT_TRUE(dsHashSizeTEqual(&val1, &val2));

	EXPECT_TRUE(dsHashSizeTEqual(nullptr, nullptr));
	EXPECT_FALSE(dsHashSizeTEqual(&val1, nullptr));
	EXPECT_FALSE(dsHashSizeTEqual(nullptr, &val2));
	EXPECT_TRUE(dsHashSizeTEqual(&val1, &val2));
}

TEST(HashTest, HashPointer)
{
	void* val1 = (void*)123;
	void* val2 = (void*)456;
	EXPECT_NE(dsHashPointer(val1), dsHashPointer(val2));
	EXPECT_EQ(dsHashBytes(&val1, sizeof(val1)), dsHashPointer(val1));
	EXPECT_EQ(dsHashCombineBytes(1234, &val1, sizeof(val1)), dsHashCombinePointer(1234, val1));

	EXPECT_FALSE(dsHashPointerEqual(val1, val2));
	val2 = val1;
	EXPECT_TRUE(dsHashPointerEqual(val1, val2));

	EXPECT_TRUE(dsHashPointerEqual(nullptr, nullptr));
	EXPECT_FALSE(dsHashPointerEqual(val1, nullptr));
	EXPECT_FALSE(dsHashPointerEqual(nullptr, val2));
	EXPECT_TRUE(dsHashPointerEqual(val1, val2));
}

TEST(HashTest, HashFloat)
{
	float val1 = 12.3f;
	float val2 = 0.0f;
	float val3 = -0.0f;
	EXPECT_NE(dsHashFloat(&val1), dsHashFloat(&val2));
	EXPECT_NE(dsHash32(&val2), dsHash32(&val3));
	EXPECT_EQ(dsHashFloat(&val2), dsHashFloat(&val3));
	EXPECT_EQ(dsHashCombineBytes(1234, &val1, sizeof(val1)), dsHashCombineFloat(1234, &val1));

	EXPECT_FALSE(dsHashFloatEqual(&val1, &val2));
	EXPECT_FALSE(dsHash32Equal(&val2, &val3));
	EXPECT_TRUE(dsHashFloatEqual(&val2, &val3));
	val2 = val1;
	EXPECT_TRUE(dsHashFloatEqual(&val1, &val2));

	EXPECT_TRUE(dsHashFloatEqual(nullptr, nullptr));
	EXPECT_FALSE(dsHashFloatEqual(&val1, nullptr));
	EXPECT_FALSE(dsHashFloatEqual(nullptr, &val2));
	EXPECT_TRUE(dsHashFloatEqual(&val1, &val2));
}

TEST(HashTest, HashDouble)
{
	double val1 = 12.3;
	double val2 = 0.0;
	double val3 = -0.0;
	EXPECT_NE(dsHashDouble(&val1), dsHashDouble(&val2));
	EXPECT_NE(dsHash64(&val2), dsHash64(&val3));
	EXPECT_EQ(dsHashDouble(&val2), dsHashDouble(&val3));
	EXPECT_EQ(dsHashCombineBytes(1234, &val1, sizeof(val1)), dsHashCombineDouble(1234, &val1));

	EXPECT_FALSE(dsHashDoubleEqual(&val1, &val2));
	EXPECT_FALSE(dsHash64Equal(&val2, &val3));
	EXPECT_TRUE(dsHashDoubleEqual(&val2, &val3));
	val2 = val1;
	EXPECT_TRUE(dsHashDoubleEqual(&val1, &val2));

	EXPECT_TRUE(dsHashDoubleEqual(nullptr, nullptr));
	EXPECT_FALSE(dsHashDoubleEqual(&val1, nullptr));
	EXPECT_FALSE(dsHashDoubleEqual(nullptr, &val2));
	EXPECT_TRUE(dsHashDoubleEqual(&val1, &val2));
}
