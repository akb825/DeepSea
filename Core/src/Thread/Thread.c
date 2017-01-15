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
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <string.h>

#if DS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#define __USE_GNU
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#endif

inline static bool isThreadSet(dsThread thread)
{
#if DS_WINDOWS
	return thread.thread != NULL;
#else
	static dsThread zeroThread;
	return memcmp(&thread.thread, &zeroThread.thread, sizeof(zeroThread.thread)) != 0;
#endif
}

static dsThreadReturnType threadWrapperFunc(void* data)
{
	dsThread* thread = (dsThread*)data;
	DS_ASSERT(thread);
	DS_ASSERT(thread->name);
	DS_ASSERT(thread->function);

	// Grab the necessary parts out of the thread, then mark it as started so it can be detached.
	const char* name = thread->name;
	dsThreadFunction function = thread->function;
	void* userData = thread->userData;

	int32_t started = true;
	DS_ATOMIC_STORE32(&thread->started, &started);

	dsThread_setThisThreadName(name);
	return function(userData);
}

bool dsThread_create(dsThread* thread, dsThreadFunction function, void* userData,
	unsigned int stackSize, const char* name)
{
	if (!thread || !function)
	{
		errno = EINVAL;
		return false;
	}

	thread->name = name ? name : "Thread";
	thread->userData = userData;
	thread->function = function;
	thread->started = false;

#if DS_WINDOWS

	HANDLE handle = CreateThread(NULL, stackSize, (LPTHREAD_START_ROUTINE)&threadWrapperFunc,
		thread, 0, NULL);
	if (!handle)
	{
		switch (GetLastError())
		{
			case ERROR_NOT_ENOUGH_MEMORY:
				errno = ENOMEM;
				break;
			case ERROR_INVALID_PARAMETER:
				errno = EINVAL;
				break;
			default:
				errno = EAGAIN;
				break;
		}
		return false;
	}

	thread->thread = handle;

#else

	pthread_attr_t attributes;
	DS_VERIFY(pthread_attr_init(&attributes) == 0);
	if (stackSize > 0)
	{
		int errorCode = pthread_attr_setstacksize(&attributes, stackSize);
		if (errorCode != 0)
		{
			DS_LOG_ERROR_F(DS_CORE_LOG_TAG, "Invalid thread stack size: %u", stackSize);
			DS_VERIFY(pthread_attr_destroy(&attributes) == 0);
			errno = errorCode;
			return false;
		}
	}

	pthread_t handle;
	int errorCode = pthread_create(&handle, &attributes, (void* (*)(void*))&threadWrapperFunc, thread);
	DS_VERIFY(pthread_attr_destroy(&attributes) == 0);

	if (errorCode != 0)
	{
		errno = errorCode;
		return false;
	}

	thread->thread = (uint64_t)handle;

#endif

	return true;
}

#if DS_WINDOWS
// From https://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx
#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
	DWORD dwType; // Must be 0x1000.
	LPCSTR szName; // Pointer to name (in user addr space).
	DWORD dwThreadID; // Thread ID (-1=caller thread).
	DWORD dwFlags; // Reserved for future use, must be zero.
 } THREADNAME_INFO;
#pragma pack(pop)
#endif

bool dsThread_setThisThreadName(const char* name)
{
#if DS_WINDOWS

	// From https://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx
	const DWORD MS_VC_EXCEPTION = 0x406D1388;
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = name;
	info.dwThreadID = GetCurrentThreadId();
	info.dwFlags = 0;
#pragma warning(push)
#pragma warning(disable: 6320 6322)
	__try
	{
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}
#pragma warning(pop)

	return true;

#elif DS_APPLE
	return pthread_setname_np(name) == 0;
#elif DS_LINUX
	int errorCode = pthread_setname_np(pthread_self(), name);
	if (errorCode != 0)
		errno = errorCode;
	return errorCode == 0;
#else
	DS_UNUSED(name);
	return false;
#endif
}

void dsThread_exit(dsThreadReturnType returnVal)
{
#if DS_WINDOWS
	ExitThread(returnVal);
#else
	pthread_exit((void*)returnVal);
#endif
}

dsThreadId dsThread_getId(dsThread thread)
{
	dsThreadId threadId;

#if DS_WINDOWS
	if (!thread.thread)
		return dsThread_invalidId();

	threadId.threadId = GetThreadId(thread.thread);
#else
	threadId.threadId = thread.thread;
#endif

	return threadId;
}

dsThreadId dsThread_thisThreadId(void)
{
	dsThreadId threadId;

#if DS_WINDOWS
	threadId.threadId = GetCurrentThreadId();
#else
	threadId.threadId = pthread_self();
#endif

	return threadId;
}

dsThreadId dsThread_invalidId(void)
{
	static dsThreadId invalidId;
	return invalidId;
}

bool dsThread_equal(dsThreadId thread1, dsThreadId thread2)
{
#if DS_WINDOWS
	return thread1.threadId == thread2.threadId;
#else
	bool thread1Set = isThreadSet(*(dsThread*)&thread1);
	bool thread2Set = isThreadSet(*(dsThread*)&thread2);
	if (!thread1Set || !thread2Set)
		return thread1Set == thread2Set;

	return pthread_equal(thread1.threadId, thread2.threadId);

#endif
}

void dsThread_yield(void)
{
#if DS_WINDOWS
	SwitchToThread();
#else
	sched_yield();
#endif
}

void dsThread_sleep(unsigned int milliseconds, const char* name)
{
	DS_PROFILE_WAIT_START(name ? name : "Sleep");

#if DS_WINDOWS
	Sleep(milliseconds);
#else
	usleep(((useconds_t)milliseconds)*1000);
#endif

	DS_PROFILE_WAIT_END();
}

bool dsThread_detach(dsThread* thread)
{
	if (!thread || !isThreadSet(*thread))
	{
		errno = EINVAL;
		return false;
	}

	// Make sure that the thread has already started. This prevents the dsThread instance from being
	// destroyed too soon.
	do
	{
		int32_t started;
		DS_ATOMIC_LOAD32(&thread->started, &started);
		if (started)
			break;

		dsThread_yield();
	} while (true);

#if DS_WINDOWS
	CloseHandle(thread->thread);
	thread->thread = 0;
#else
	int errorCode = pthread_detach(thread->thread);
	if (errorCode != 0)
	{
		errno = errorCode;
		return false;
	}
	memset(&thread->thread, 0, sizeof(thread->thread));
#endif

	return true;
}

bool dsThread_join(dsThread* thread, dsThreadReturnType* returnVal)
{
	if (!thread || !isThreadSet(*thread))
	{
		errno = EINVAL;
		return false;
	}

	DS_ASSERT(thread->name);
	DS_PROFILE_WAIT_START(thread->name);

	bool result;

#if DS_WINDOWS

	result = WaitForSingleObject(thread->thread, INFINITE) != WAIT_FAILED;

	if (result)
	{
		if (returnVal)
			DS_VERIFY(GetExitCodeThread(thread->thread, (DWORD*)returnVal));

		CloseHandle(thread->thread);
		thread->thread = 0;
	}
	else
		errno = EINVAL;

#else

	int errorCode = pthread_join(thread->thread, (void**)returnVal);
	if (errorCode != 0)
		errno = errorCode;
	result = errorCode == 0;

	if (result)
		memset(&thread->thread, 0, sizeof(thread->thread));

#endif

	DS_PROFILE_WAIT_END();
	return result;
}
