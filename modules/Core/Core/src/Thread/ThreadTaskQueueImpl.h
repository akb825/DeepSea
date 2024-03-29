/*
 * Copyright 2023 Aaron Barany
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

#pragma once

#include <DeepSea/Core/Types.h>

typedef struct dsThreadTaskEntry dsThreadTaskEntry;

struct dsThreadTaskEntry
{
	dsThreadTaskEntry* next;
	dsThreadTask task;
};

struct dsThreadTaskQueue
{
	dsAllocator* allocator;

	dsThreadPool* threadPool;
	dsPoolAllocator taskAllocator;
	dsThreadTaskEntry* taskHead;
	dsThreadTaskEntry* taskTail;
	uint32_t maxConcurrency;
	uint32_t executingTasks;
	dsSpinlock addTaskLock;
	dsConditionVariable* finishTasksCondition;
};

bool dsThreadTaskQueue_popTask(dsThreadTask* outTask, dsThreadTaskQueue* taskQueue);
void dsThreadTaskQueue_finishTask(dsThreadTaskQueue* taskQueue);
