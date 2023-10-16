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

#include "Helpers.h"
#include <DeepSea/Core/Thread/ConditionVariable.h>
#include <DeepSea/Core/Thread/Mutex.h>
#include <DeepSea/Core/Thread/Thread.h>
#include <DeepSea/Core/Timer.h>
#include <thread>
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
	dsThreadID threadID;
	dsConditionVariable* condition;
	dsMutex* mutex;
	bool ready;
};

dsThreadReturnType threadID(void* data)
{
	ThreadIdData* threadIDData = (ThreadIdData*)data;
	EXPECT_TRUE(dsMutex_lock(threadIDData->mutex));
	while (!threadIDData->ready)
	{
		EXPECT_EQ(dsConditionVariableResult_Success, dsConditionVariable_wait(
			threadIDData->condition, threadIDData->mutex));
	}
	EXPECT_TRUE(dsMutex_unlock(threadIDData->mutex));

	EXPECT_TRUE(dsThread_equal(dsThread_thisThreadID(), threadIDData->threadID));
	return 0;
}

} // namespace

TEST(Thread, LogicalCoreCount)
{
	EXPECT_EQ(std::thread::hardware_concurrency(), dsThread_logicalCoreCount());
}

TEST(Thread, Join)
{
	EXPECT_FALSE_ERRNO(EINVAL, dsThread_create(nullptr, nullptr, nullptr, 0, nullptr));

	dsThread thread1, thread2, thread3, thread4;
	dsThreadReturnType ret1, ret2, ret3;

	EXPECT_FALSE_ERRNO(EINVAL, dsThread_create(&thread1, nullptr, nullptr, 0, nullptr));
	EXPECT_TRUE(dsThread_create(&thread1, &simpleThread, (void*)1, 0, nullptr));
	EXPECT_TRUE(dsThread_create(&thread2, &simpleThread, (void*)2, 0, nullptr));
	EXPECT_TRUE(dsThread_create(&thread3, &simpleThread, (void*)3, 0, nullptr));
	EXPECT_TRUE(dsThread_create(&thread4, &simpleThread, (void*)4, 0, nullptr));

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
	EXPECT_TRUE(dsThread_create(&thread, &simpleThread, (void*)1, 0, nullptr));
	EXPECT_TRUE(dsThread_detach(&thread));
	EXPECT_FALSE_ERRNO(EINVAL, dsThread_detach(&thread));
	EXPECT_FALSE_ERRNO(EINVAL, dsThread_join(&thread, NULL));
	EXPECT_FALSE_ERRNO(EINVAL, dsThread_detach(NULL));
}

TEST(Thread, ExitThread)
{
	dsThread thread1, thread2, thread3;
	dsThreadReturnType ret1, ret2, ret3;

	EXPECT_TRUE(dsThread_create(&thread1, &exitThread, (void*)1, 0, nullptr));
	EXPECT_TRUE(dsThread_create(&thread2, &exitThread, (void*)2, 0, nullptr));
	EXPECT_TRUE(dsThread_create(&thread3, &exitThread, (void*)3, 0, nullptr));

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

	EXPECT_TRUE(dsThread_create(&thread1, &namedThread, (void*)"Thread 1", 0, nullptr));
	EXPECT_TRUE(dsThread_create(&thread2, &namedThread, (void*)"Thread 2", 0, nullptr));
	EXPECT_TRUE(dsThread_create(&thread3, &namedThread, (void*)"Thread 3", 0, nullptr));

	EXPECT_TRUE(dsThread_join(&thread1, NULL));
	EXPECT_TRUE(dsThread_join(&thread2, NULL));
	EXPECT_TRUE(dsThread_join(&thread3, NULL));
}

TEST(Thread, ThreadId)
{
	EXPECT_TRUE(dsThread_equal(dsThread_invalidID(), dsThread_invalidID()));
	EXPECT_FALSE(dsThread_equal(dsThread_thisThreadID(), dsThread_invalidID()));
	EXPECT_FALSE(dsThread_equal(dsThread_invalidID(), dsThread_thisThreadID()));
	EXPECT_TRUE(dsThread_equal(dsThread_thisThreadID(), dsThread_thisThreadID()));

	dsConditionVariable* condition = dsConditionVariable_create(nullptr, nullptr);
	ASSERT_NE(nullptr, condition);
	dsMutex* mutex = dsMutex_create(nullptr, nullptr);
	ASSERT_NE(nullptr, mutex);

	dsThread thread1, thread2, thread3;
	ThreadIdData data1 = {}, data2 = {}, data3 = {};

	data1.condition = condition;
	data1.mutex = mutex;
	data2.condition = condition;
	data2.mutex = mutex;
	data3.condition = condition;
	data3.mutex = mutex;

	EXPECT_TRUE(dsThread_create(&thread1, &threadID, &data1, 0, nullptr));
	EXPECT_TRUE(dsThread_create(&thread2, &threadID, &data2, 0, nullptr));
	EXPECT_TRUE(dsThread_create(&thread3, &threadID, &data3, 0, nullptr));

	data1.threadID = dsThread_getID(&thread1);
	data2.threadID = dsThread_getID(&thread2);
	data3.threadID = dsThread_getID(&thread3);

	EXPECT_TRUE(dsMutex_lock(mutex));
	data1.ready = data2.ready = data3.ready = true;
	EXPECT_TRUE(dsConditionVariable_notifyAll(condition));
	EXPECT_TRUE(dsMutex_unlock(mutex));

	EXPECT_TRUE(dsThread_join(&thread1, NULL));
	EXPECT_TRUE(dsThread_join(&thread2, NULL));
	EXPECT_TRUE(dsThread_join(&thread3, NULL));

	dsConditionVariable_destroy(condition);
	dsMutex_destroy(mutex);
}

TEST(Thread, Sleep)
{
	dsTimer timer = dsTimer_create();
	double startTime = dsTimer_time(timer);
	dsThread_sleep(150, nullptr);
	double endTime = dsTimer_time(timer);
	// Give a generous error due to scheduling quantums.
	EXPECT_NEAR(150, (endTime - startTime)*1000, 20);
}
