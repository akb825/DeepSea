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

#include <DeepSea/Core/Thread/Thread.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Timer.h>
#include <gtest/gtest.h>

namespace
{

#if DS_MSC
#pragma warning(push)
#pragma warning(disable: 4302 4311)
#endif

dsThreadReturnType getReturnValue(void* data)
{
	return (dsThreadReturnType)data;
}

#if DS_MSC
#pragma warning(pop)
#endif

dsThreadReturnType simpleThread(void* data)
{
	return getReturnValue(data);
}

dsThreadReturnType exitThread(void* data)
{
	dsThread_exit(getReturnValue(data));
	EXPECT_TRUE(false);
	return 0;
}

dsThreadReturnType namedThread(void* data)
{
	dsThread_setThisThreadName((const char*)data);
	return 0;
}

struct ThreadIdData
{
	uint32_t ready;
	dsThreadId threadId;
};

dsThreadReturnType threadId(void* data)
{
	ThreadIdData* threadIdData = (ThreadIdData*)data;
	uint32_t ready;
	do
	{
		DS_ATOMIC_LOAD32(&threadIdData->ready, &ready);
	} while (!ready);

	EXPECT_TRUE(dsThread_equal(dsThread_thisThreadId(), threadIdData->threadId));
	return 0;
}

} // namespace

TEST(Thread, Join)
{
	EXPECT_FALSE(dsThread_create(nullptr, nullptr, nullptr, 0));

	dsThread thread1, thread2, thread3, thread4;
	dsThreadReturnType ret1, ret2, ret3;

	EXPECT_FALSE(dsThread_create(&thread1, nullptr, nullptr, 0));
	EXPECT_TRUE(dsThread_create(&thread1, &simpleThread, (void*)1, 0));
	EXPECT_TRUE(dsThread_create(&thread2, &simpleThread, (void*)2, 0));
	EXPECT_TRUE(dsThread_create(&thread3, &simpleThread, (void*)3, 0));
	EXPECT_TRUE(dsThread_create(&thread4, &simpleThread, (void*)4, 0));

	EXPECT_TRUE(dsThread_join(&thread1, &ret1));
	EXPECT_EQ(1, ret1);
	EXPECT_TRUE(dsThread_join(&thread2, &ret2));
	EXPECT_EQ(2, ret2);
	EXPECT_TRUE(dsThread_join(&thread3, &ret3));
	EXPECT_EQ(3, ret3);
	EXPECT_TRUE(dsThread_join(&thread4, NULL));

	EXPECT_FALSE(dsThread_join(&thread1, &ret1));
	EXPECT_FALSE(dsThread_join(NULL, &ret1));
	EXPECT_FALSE(dsThread_join(&thread2, &ret2));
	EXPECT_FALSE(dsThread_join(&thread3, &ret3));
	EXPECT_FALSE(dsThread_join(&thread4, NULL));
	EXPECT_EQ(1, ret1);
	EXPECT_EQ(2, ret2);
	EXPECT_EQ(3, ret3);
}

TEST(Thread, Detach)
{
	dsThread thread;
	EXPECT_TRUE(dsThread_create(&thread, &simpleThread, (void*)1, 0));
	EXPECT_TRUE(dsThread_detach(&thread));
	EXPECT_FALSE(dsThread_detach(&thread));
	EXPECT_FALSE(dsThread_join(&thread, NULL));
	EXPECT_FALSE(dsThread_detach(NULL));
}

TEST(Thread, ExitThread)
{
	dsThread thread1, thread2, thread3;
	dsThreadReturnType ret1, ret2, ret3;

	EXPECT_TRUE(dsThread_create(&thread1, &exitThread, (void*)1, 0));
	EXPECT_TRUE(dsThread_create(&thread2, &exitThread, (void*)2, 0));
	EXPECT_TRUE(dsThread_create(&thread3, &exitThread, (void*)3, 0));

	EXPECT_TRUE(dsThread_join(&thread1, &ret1));
	EXPECT_EQ(1, ret1);
	EXPECT_TRUE(dsThread_join(&thread2, &ret2));
	EXPECT_EQ(2, ret2);
	EXPECT_TRUE(dsThread_join(&thread3, &ret3));
	EXPECT_EQ(3, ret3);
}

TEST(Thread, NameThread)
{
	dsThread thread1, thread2, thread3;

	EXPECT_TRUE(dsThread_create(&thread1, &namedThread, (void*)"Thread 1", 0));
	EXPECT_TRUE(dsThread_create(&thread2, &namedThread, (void*)"Thread 2", 0));
	EXPECT_TRUE(dsThread_create(&thread3, &namedThread, (void*)"Thread 3", 0));

	EXPECT_TRUE(dsThread_join(&thread1, NULL));
	EXPECT_TRUE(dsThread_join(&thread2, NULL));
	EXPECT_TRUE(dsThread_join(&thread3, NULL));
}

TEST(Thread, ThreadId)
{
	EXPECT_TRUE(dsThread_equal(dsThread_invalidId(), dsThread_invalidId()));
	EXPECT_FALSE(dsThread_equal(dsThread_thisThreadId(), dsThread_invalidId()));
	EXPECT_FALSE(dsThread_equal(dsThread_invalidId(), dsThread_thisThreadId()));
	EXPECT_TRUE(dsThread_equal(dsThread_thisThreadId(), dsThread_thisThreadId()));

	dsThread thread1, thread2, thread3;
	ThreadIdData data1 = {}, data2 = {}, data3 = {};
	uint32_t ready = true;

	EXPECT_TRUE(dsThread_create(&thread1, &threadId, &data1, 0));
	EXPECT_TRUE(dsThread_create(&thread2, &threadId, &data2, 0));
	EXPECT_TRUE(dsThread_create(&thread3, &threadId, &data3, 0));

	data1.threadId = dsThread_getId(thread1);
	DS_ATOMIC_STORE32(&data1.ready, &ready);
	data2.threadId = dsThread_getId(thread2);
	DS_ATOMIC_STORE32(&data2.ready, &ready);
	data3.threadId = dsThread_getId(thread3);
	DS_ATOMIC_STORE32(&data3.ready, &ready);

	EXPECT_TRUE(dsThread_join(&thread1, NULL));
	EXPECT_TRUE(dsThread_join(&thread2, NULL));
	EXPECT_TRUE(dsThread_join(&thread3, NULL));
}

TEST(Thread, DISABLED_Sleep)
{
	dsTimer timer = dsTimer_create();
	double startTime = dsTimer_getTime(timer);
	dsThread_sleep(150);
	double endTime = dsTimer_getTime(timer);
	// Give a generous error due to scheduling quantums.
	EXPECT_NEAR(150, (endTime - startTime)*1000, 20);
}
