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

#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Thread/ThreadStorage.h>
#include <string.h>

#define BUFFER_SIZE 256
static DS_THREAD_LOCAL char buffer[BUFFER_SIZE];

const char* dsErrorString(int errorCode)
{
#if DS_WINDOWS
	DS_VERIFY(strerror_s(buffer, BUFFER_SIZE, errorCode) == 0);
#elif defined(_GNU_SOURCE)
	DS_VERIFY(strerror_r(errorCode, buffer, BUFFER_SIZE) == buffer);
#else
	DS_VERIFY(strerror_r(errorCode, buffer, BUFFER_SIZE) == 0);
#endif
	return buffer;
}
