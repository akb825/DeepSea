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
#include <DeepSea/Core/Log.h>
#include <string.h>

#if DS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#define __USE_GNU
#include <pthread.h>
#include <unistd.h>
#endif

bool dsThread_create(dsThread* thread, dsThreadFunction function, void* userData,
	unsigned int stackSize)
{
	if (!thread)
		return false;

#if DS_WINDOWS

	HANDLE handle = CreateThread(NULL, stackSize, (LPTHREAD_START_ROUTINE)function, userData, 0,
		NULL);
	if (!handle)
		return false;

	thread->thread = handle;

#else

	pthread_attr_t attributes;
	DS_VERIFY(pthread_attr_init(&attributes) == 0);
	if (stackSize > 0)
	{
		if (pthread_attr_setstacksize(&attributes, stackSize) != 0)
		{
			DS_LOG_ERROR_F("thread", "Invalid thread stack size: %u", stackSize);
			DS_VERIFY(pthread_attr_destroy(&attributes) == 0);
			return false;
		}
	}

	pthread_t handle;
	bool success = pthread_create(&handle, &attributes, (void* (*)(void*))function, userData) == 0;
	DS_VERIFY(pthread_attr_destroy(&attributes) == 0);

	if (!success)
		return false;

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

#elif DS_APPLE
	return pthread_setname_np(name) == 0;
#elif DS_LINUX
	return pthread_setname_np(pthread_self(), name) == 0;
#else
	DS_UNUSED(name);
	return false;
#endif

	return true;
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

dsThreadId dsThread_thisThreadId()
{
	dsThreadId threadId;

#if DS_WINDOWS
	threadId.threadId = GetCurrentThreadId();
#else
	threadId.threadId = pthread_self();
#endif

	return threadId;
}

dsThreadId dsThread_invalidId()
{
	static dsThreadId invalidId;
	return invalidId;
}

bool dsThread_equal(dsThreadId thread1, dsThreadId thread2)
{
#if DS_WINDOWS
	return thread1.threadId == thread2.threadId;
#else
	return pthread_equal(thread1.threadId, thread2.threadId);
#endif
}

void dsThread_sleep(unsigned int milliseconds)
{
#if DS_WINDOWS
	Sleep(milliseconds);
#else
	usleep(((useconds_t)milliseconds)*1000);
#endif
}

bool dsThread_detach(dsThread* thread)
{
	if (!thread)
		return false;

#if DS_WINDOWS

	if (!thread->thread)
		return false;
	CloseHandle(thread->thread);
	thread->thread = 0;

#else
	if (pthread_detach(thread->thread) != 0)
		return false;
	memset(&thread->thread, 0, sizeof(thread->thread));
#endif

	return true;
}

bool dsThread_join(dsThread* thread, dsThreadReturnType* returnVal)
{
	if (!thread)
		return false;

#if DS_WINDOWS

	if (!thread->thread)
		return false;

	if (WaitForSingleObject(thread->thread, INFINITE) == WAIT_FAILED)
		return false;

	if (returnVal)
		DS_VERIFY(GetExitCodeThread(thread->thread, (DWORD*)returnVal));

	CloseHandle(thread->thread);
	thread->thread = 0;

#else
	if (pthread_join(thread->thread, (void**)returnVal) != 0)
		return false;
	memset(&thread->thread, 0, sizeof(thread->thread));
#endif

	return true;
}
