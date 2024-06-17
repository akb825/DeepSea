/*
 * Copyright 2024 Aaron Barany
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

#include <DeepSea/Core/Thread/ThreadObjectStorage.h>

#include <DeepSea/Core/Containers/List.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#if DS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <pthread.h>
#endif

#if DS_WINDOWS

// Need to wrap for different callback calling conventions.
typedef struct StorageWrapper
{
	dsThreadObjectStorage* parent;
	void* object;
} StorageWrapper;

#else

// With pthreads we need to maintain the active objects ourselves to clean up on
// dsThreadObjectStorage destruction.
typedef struct StorageNode
{
	dsListNode node;
	dsThreadObjectStorage* parent;
	void* object;
} StorageNode;

#endif

struct dsThreadObjectStorage
{
	dsAllocator* allocator;
	dsDestroyUserDataFunction cleanupFunc;
#if DS_WINDOWS
	DWORD storage;
#else
	dsList list;
	dsSpinlock lock;
	pthread_key_t storage;
#endif
};

#if DS_WINDOWS

static void NTAPI onThreadExit(void* data)
{
	StorageWrapper* wrapper = (StorageWrapper*)data;
	dsThreadObjectStorage* storage = wrapper->parent;
	if (wrapper->object)
		storage->cleanupFunc(wrapper->object);
	DS_VERIFY(dsAllocator_free(storage->allocator, wrapper));
}

#else

static void onThreadExit(void* data)
{
	StorageNode* node = (StorageNode*)data;
	dsThreadObjectStorage* storage = node->parent;
	DS_VERIFY(dsSpinlock_lock(&storage->lock));
	DS_VERIFY(dsList_remove(&storage->list, (dsListNode*)node));
	DS_VERIFY(dsSpinlock_unlock(&storage->lock));

	if (node->object)
		storage->cleanupFunc(node->object);
	DS_VERIFY(dsAllocator_free(storage->allocator, node));
}

#endif

size_t dsThreadObjectStorage_sizeof(void)
{
	return sizeof(dsThreadObjectStorage);
}

dsThreadObjectStorage* dsThreadObjectStorage_create(dsAllocator* allocator,
	dsDestroyUserDataFunction cleanupFunc)
{
	if (!allocator || !cleanupFunc)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		DS_LOG_ERROR(DS_CORE_LOG_TAG,
			"Thread object storage allocator must support freeing memory.");
		errno = EINVAL;
		return NULL;
	}

#if DS_WINDOWS

	DWORD baseStorage = FlsAlloc(onThreadExit);
	if (baseStorage == FLS_OUT_OF_INDEXES)
	{
		if (GetLastError() == ERROR_NOT_ENOUGH_MEMORY)
			errno = ENOMEM;
		else
			errno = EAGAIN;
		return NULL;
	}


#else

	pthread_key_t baseStorage;
	int errorCode = pthread_key_create(&baseStorage, &onThreadExit);
	if (errorCode != 0)
	{
		errno = errorCode;
		return NULL;
	}

#endif

	dsThreadObjectStorage* storage = DS_ALLOCATE_OBJECT(allocator, dsThreadObjectStorage);
	if (!storage)
	{
#if DS_WINDOWS
		FlsFree(baseStorage);
#else
		pthread_key_delete(baseStorage);
#endif
		return NULL;
	}

	storage->allocator = dsAllocator_keepPointer(allocator);
	storage->cleanupFunc = cleanupFunc;
#if !DS_WINDOWS
	DS_VERIFY(dsList_initialize(&storage->list));
	DS_VERIFY(dsSpinlock_initialize(&storage->lock));
#endif
	storage->storage = baseStorage;
	return storage;
}

void* dsThreadObjectStorage_get(const dsThreadObjectStorage* storage)
{
	if (!storage)
		return NULL;

#if DS_WINDOWS

	StorageWrapper* wrapper = (StorageWrapper*)FlsGetValue(storage->storage);
	return wrapper ? wrapper->object : NULL;

#else

	StorageNode* node = (StorageNode*)pthread_getspecific(storage->storage);
	return node ? node->object : NULL;

#endif
}

void* dsThreadObjectStorage_take(const dsThreadObjectStorage* storage)
{
	if (!storage)
		return NULL;

#if DS_WINDOWS

	StorageWrapper* wrapper = (StorageWrapper*)FlsGetValue(storage->storage);
	if (wrapper)
	{
		void* orig = wrapper->object;
		wrapper->object = NULL;
		return orig;
	}
	return NULL;

#else

	StorageNode* node = (StorageNode*)pthread_getspecific(storage->storage);
	if (node)
	{
		void* orig = node->object;
		node->object = NULL;
		return orig;
	}
	return NULL;

#endif

}

bool dsThreadObjectStorage_set(dsThreadObjectStorage* storage, void* object)
{
	if (!storage)
	{
		errno = EINVAL;
		return false;
	}

#if DS_WINDOWS

	StorageWrapper* wrapper = (StorageWrapper*)FlsGetValue(storage->storage);
	if (wrapper)
	{
		if (wrapper->object && wrapper->object != object)
			storage->cleanupFunc(wrapper->object);
		wrapper->object = object;
		return true;
	}

	wrapper = DS_ALLOCATE_OBJECT(storage->allocator, StorageWrapper);
	if (!wrapper)
		return false;

	wrapper->parent = storage;
	wrapper->object = object;

	if (!FlsSetValue(storage->storage, wrapper))
	{
		dsAllocator_free(storage->allocator, wrapper);
		errno = EINVAL;
		return false;
	}
	return true;

#else

	StorageNode* node = (StorageNode*)pthread_getspecific(storage->storage);
	if (node)
	{
		if (node->object && node->object != object)
			storage->cleanupFunc(node->object);
		node->object = object;
		return true;
	}

	node = DS_ALLOCATE_OBJECT(storage->allocator, StorageNode);
	if (!node)
		return false;

	node->parent = storage;
	node->object = object;

	DS_VERIFY(dsSpinlock_lock(&storage->lock));
	DS_VERIFY(dsList_append(&storage->list, (dsListNode*)node));
	DS_VERIFY(dsSpinlock_unlock(&storage->lock));
	int errorCode = pthread_setspecific(storage->storage, node);
	if (errorCode != 0)
	{
		dsAllocator_free(storage->allocator, node);
		errno = errorCode;
		return false;
	}
	return true;

#endif
}

void dsThreadObjectStorage_destroy(dsThreadObjectStorage* storage)
{
	if (!storage)
		return;

#if DS_WINDOWS
	FlsFree(storage->storage);
#else

	pthread_key_delete(storage->storage);
	dsSpinlock_shutdown(&storage->lock);

	dsListNode* node = storage->list.head;
	while (node)
	{
		dsListNode* next = node->next;
		void* object = ((StorageNode*)node)->object;
		if (object)
			storage->cleanupFunc(object);
		DS_VERIFY(dsAllocator_free(storage->allocator, node));
		node = next;
	}

#endif

	DS_VERIFY(dsAllocator_free(storage->allocator, storage));
}
