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

#include <DeepSea/Core/Thread/ThreadStorage.h>
#include <string.h>

#if DS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <pthread.h>
#endif

bool dsThreadStorage_initialize(dsThreadStorage* storage)
{
	if (!storage)
		return false;

#if DS_WINDOWS

	DWORD tls = TlsAlloc();
	if (tls == 0)
		return false;

	storage->storage = tls;
	return true;

#else
	return pthread_key_create(&storage->storage, NULL) == 0;
#endif
}

void* dsThreadStorage_get(dsThreadStorage storage)
{
#if DS_WINDOWS
	return TlsGetValue(storage.storage);
#else
	return pthread_getspecific(storage.storage);
#endif
}

bool dsThreadStorage_set(dsThreadStorage storage, void* value)
{
#if DS_WINDOWS
	return TlsSetValue(storage.storage, value);
#else
	return pthread_setspecific(storage.storage, value) == 0;
#endif
}

void dsThreadStorage_destroy(dsThreadStorage* storage)
{
#if DS_WINDOWS
	TlsFree(storage->storage);
	storage->storage = 0;
#else
	pthread_key_delete(storage->storage);
	memset(&storage->storage, 0, sizeof(storage->storage));
#endif
}
