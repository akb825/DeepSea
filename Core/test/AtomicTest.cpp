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

#include <DeepSea/Core/Atomic.h>
#include <gtest/gtest.h>

TEST(Atomic, int32_t)
{
	int32_t atomicVal = 15;
	int32_t testVal, exchangeVal;

	DS_ATOMIC_LOAD32(&atomicVal, &testVal);
	EXPECT_EQ(15, testVal);

	testVal = 20;
	DS_ATOMIC_STORE32(&atomicVal, &testVal);
	EXPECT_EQ(20, atomicVal);

	exchangeVal = 0;
	testVal = 25;
	DS_ATOMIC_EXCHANGE32(&atomicVal, &testVal, &exchangeVal);
	EXPECT_EQ(25, atomicVal);
	EXPECT_EQ(20, exchangeVal);

	testVal = 30;
	exchangeVal = 20;
	EXPECT_FALSE(DS_ATOMIC_COMPARE_EXCHANGE32(&atomicVal, &exchangeVal, &testVal, false));
	EXPECT_EQ(25, atomicVal);
	EXPECT_EQ(25, exchangeVal);

	EXPECT_TRUE(DS_ATOMIC_COMPARE_EXCHANGE32(&atomicVal, &exchangeVal, &testVal, false));
	EXPECT_EQ(30, atomicVal);
	EXPECT_EQ(25, exchangeVal);

	EXPECT_EQ(30, DS_ATOMIC_FETCH_ADD32(&atomicVal, -3));
	EXPECT_EQ(27, atomicVal);
}

TEST(Atomic, uint32_t)
{
	uint32_t atomicVal = 15;
	uint32_t testVal, exchangeVal;

	DS_ATOMIC_LOAD32(&atomicVal, &testVal);
	EXPECT_EQ(15U, testVal);

	testVal = 20;
	DS_ATOMIC_STORE32(&atomicVal, &testVal);
	EXPECT_EQ(20U, atomicVal);

	exchangeVal = 0;
	testVal = 25;
	DS_ATOMIC_EXCHANGE32(&atomicVal, &testVal, &exchangeVal);
	EXPECT_EQ(25U, atomicVal);
	EXPECT_EQ(20U, exchangeVal);

	testVal = 30;
	exchangeVal = 20;
	EXPECT_FALSE(DS_ATOMIC_COMPARE_EXCHANGE32(&atomicVal, &exchangeVal, &testVal, false));
	EXPECT_EQ(25U, atomicVal);
	EXPECT_EQ(25U, exchangeVal);

	EXPECT_TRUE(DS_ATOMIC_COMPARE_EXCHANGE32(&atomicVal, &exchangeVal, &testVal, false));
	EXPECT_EQ(30U, atomicVal);
	EXPECT_EQ(25U, exchangeVal);

	EXPECT_EQ(30, DS_ATOMIC_FETCH_ADD32(&atomicVal, -3));
	EXPECT_EQ(27U, atomicVal);
}

TEST(Atomic, float)
{
	float atomicVal = 1.5f;
	float testVal, exchangeVal;

	DS_ATOMIC_LOAD32(&atomicVal, &testVal);
	EXPECT_EQ(1.5f, testVal);

	testVal = 2.0f;
	DS_ATOMIC_STORE32(&atomicVal, &testVal);
	EXPECT_EQ(2.0f, atomicVal);

	exchangeVal = 0;
	testVal = 2.5f;
	DS_ATOMIC_EXCHANGE32(&atomicVal, &testVal, &exchangeVal);
	EXPECT_EQ(2.5f, atomicVal);
	EXPECT_EQ(2.0f, exchangeVal);

	testVal = 3.0f;
	exchangeVal = 2.0f;
	EXPECT_FALSE(DS_ATOMIC_COMPARE_EXCHANGE32(&atomicVal, &exchangeVal, &testVal, false));
	EXPECT_EQ(2.5f, atomicVal);
	EXPECT_EQ(2.5f, exchangeVal);

	EXPECT_TRUE(DS_ATOMIC_COMPARE_EXCHANGE32(&atomicVal, &exchangeVal, &testVal, false));
	EXPECT_EQ(3.0f, atomicVal);
	EXPECT_EQ(2.5f, exchangeVal);
}

TEST(Atomic, int64_t)
{
	int64_t atomicVal = 15;
	int64_t testVal, exchangeVal;

	DS_ATOMIC_LOAD64(&atomicVal, &testVal);
	EXPECT_EQ(15, testVal);

	testVal = 20;
	DS_ATOMIC_STORE64(&atomicVal, &testVal);
	EXPECT_EQ(20, atomicVal);

	exchangeVal = 0;
	testVal = 25;
	DS_ATOMIC_EXCHANGE64(&atomicVal, &testVal, &exchangeVal);
	EXPECT_EQ(25, atomicVal);
	EXPECT_EQ(20, exchangeVal);

	testVal = 30;
	exchangeVal = 20;
	EXPECT_FALSE(DS_ATOMIC_COMPARE_EXCHANGE64(&atomicVal, &exchangeVal, &testVal, false));
	EXPECT_EQ(25, atomicVal);
	EXPECT_EQ(25, exchangeVal);

	EXPECT_TRUE(DS_ATOMIC_COMPARE_EXCHANGE64(&atomicVal, &exchangeVal, &testVal, false));
	EXPECT_EQ(30, atomicVal);
	EXPECT_EQ(25, exchangeVal);

	EXPECT_EQ(30, DS_ATOMIC_FETCH_ADD64(&atomicVal, -3));
	EXPECT_EQ(27, atomicVal);
}

TEST(Atomic, uint64_t)
{
	uint64_t atomicVal = 15;
	uint64_t testVal, exchangeVal;

	DS_ATOMIC_LOAD64(&atomicVal, &testVal);
	EXPECT_EQ(15U, testVal);

	testVal = 20;
	DS_ATOMIC_STORE64(&atomicVal, &testVal);
	EXPECT_EQ(20U, atomicVal);

	exchangeVal = 0;
	testVal = 25;
	DS_ATOMIC_EXCHANGE64(&atomicVal, &testVal, &exchangeVal);
	EXPECT_EQ(25U, atomicVal);
	EXPECT_EQ(20U, exchangeVal);

	testVal = 30;
	exchangeVal = 20;
	EXPECT_FALSE(DS_ATOMIC_COMPARE_EXCHANGE64(&atomicVal, &exchangeVal, &testVal, false));
	EXPECT_EQ(25U, atomicVal);
	EXPECT_EQ(25U, exchangeVal);

	EXPECT_TRUE(DS_ATOMIC_COMPARE_EXCHANGE64(&atomicVal, &exchangeVal, &testVal, false));
	EXPECT_EQ(30U, atomicVal);
	EXPECT_EQ(25U, exchangeVal);

	EXPECT_EQ(30, DS_ATOMIC_FETCH_ADD64(&atomicVal, -3));
	EXPECT_EQ(27U, atomicVal);
}

TEST(Atomic, double)
{
	double atomicVal = 1.5;
	double testVal, exchangeVal;

	DS_ATOMIC_LOAD64(&atomicVal, &testVal);
	EXPECT_EQ(1.5, testVal);

	testVal = 2.0;
	DS_ATOMIC_STORE64(&atomicVal, &testVal);
	EXPECT_EQ(2.0f, atomicVal);

	exchangeVal = 0;
	testVal = 2.5;
	DS_ATOMIC_EXCHANGE64(&atomicVal, &testVal, &exchangeVal);
	EXPECT_EQ(2.5, atomicVal);
	EXPECT_EQ(2.0, exchangeVal);

	testVal = 3.0;
	exchangeVal = 2.0;
	EXPECT_FALSE(DS_ATOMIC_COMPARE_EXCHANGE64(&atomicVal, &exchangeVal, &testVal, false));
	EXPECT_EQ(2.5, atomicVal);
	EXPECT_EQ(2.5, exchangeVal);

	EXPECT_TRUE(DS_ATOMIC_COMPARE_EXCHANGE64(&atomicVal, &exchangeVal, &testVal, false));
	EXPECT_EQ(3.0, atomicVal);
	EXPECT_EQ(2.5, exchangeVal);
}

TEST(Atomic, Pointer)
{
	int32_t* atomicVal = (int32_t*)16;
	int32_t* testVal;
	int32_t* exchangeVal;

	DS_ATOMIC_LOADPTR(&atomicVal, &testVal);
	EXPECT_EQ((int32_t*)16, testVal);

	testVal = (int32_t*)20;
	DS_ATOMIC_STOREPTR(&atomicVal, &testVal);
	EXPECT_EQ((int32_t*)20, atomicVal);

	exchangeVal = 0;
	testVal = (int32_t*)24;
	DS_ATOMIC_EXCHANGEPTR(&atomicVal, &testVal, &exchangeVal);
	EXPECT_EQ((int32_t*)24, atomicVal);
	EXPECT_EQ((int32_t*)20, exchangeVal);

	testVal = (int32_t*)28;
	exchangeVal = (int32_t*)20;
	EXPECT_FALSE(DS_ATOMIC_COMPARE_EXCHANGEPTR(&atomicVal, &exchangeVal, &testVal, false));
	EXPECT_EQ((int32_t*)24, atomicVal);
	EXPECT_EQ((int32_t*)24, exchangeVal);

	EXPECT_TRUE(DS_ATOMIC_COMPARE_EXCHANGEPTR(&atomicVal, &exchangeVal, &testVal, false));
	EXPECT_EQ((int32_t*)28, atomicVal);
	EXPECT_EQ((int32_t*)24, exchangeVal);

	EXPECT_EQ((int32_t*)28, DS_ATOMIC_FETCH_ADDPTR(&atomicVal, -3));
	EXPECT_EQ((int32_t*)16, atomicVal);
}
