/*
 * Copyright 2018 Aaron Barany
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

#include "VkShared.h"
#include <DeepSea/Core/Thread/ThreadStorage.h>
#include <DeepSea/Core/Error.h>

typedef struct LastCallsite
{
	const char* lastFile;
	const char* lastFunction;
	unsigned int lastLine;
} LastCallsite;

static DS_THREAD_LOCAL LastCallsite lastCallsite;

bool dsHandleVkResult(VkResult result)
{
	switch (result)
	{
		case VK_SUCCESS:
			return true;
		case VK_ERROR_OUT_OF_HOST_MEMORY:
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		case VK_ERROR_FRAGMENTED_POOL:
		case VK_ERROR_OUT_OF_POOL_MEMORY:
			errno = ENOMEM;
			return false;
		default:
			errno = EPERM;
			return false;
	}
}

void dsSetLastVkCallsite(const char* file, const char* function, unsigned int line)
{
	LastCallsite* curLastCallsite = &lastCallsite;
	curLastCallsite->lastFile = file;
	curLastCallsite->lastFunction = function;
	curLastCallsite->lastLine = line;
}

void dsGetLastVkCallsite(const char** file, const char** function, unsigned int* line)
{
	const LastCallsite* curLastCallsite = &lastCallsite;
	*file = curLastCallsite->lastFile;
	*function = curLastCallsite->lastFunction;
	*line = curLastCallsite->lastLine;
}
