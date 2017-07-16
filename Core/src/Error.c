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

#include <DeepSea/Core/Thread/ThreadStorage.h>
#include <DeepSea/Core/Log.h>
#include <string.h>

#define BUFFER_SIZE 256
static DS_THREAD_LOCAL char buffer[BUFFER_SIZE];

#define FIRST_CUSTOM EINDEX
#define LAST_CUSTOM EFORMAT

static const char* customCodes[] =
{
	"Index out of range",
	"Invalid size",
	"Invalid file format"
};

const char* dsErrorString(int errorCode)
{
	if (errorCode >= FIRST_CUSTOM && errorCode <= LAST_CUSTOM)
		return customCodes[errorCode - FIRST_CUSTOM];

#if DS_WINDOWS
	if (strerror_s(buffer, BUFFER_SIZE, errorCode) != 0)
		return "Unknown error";
#elif defined(_GNU_SOURCE)
	if (strerror_r(errorCode, buffer, BUFFER_SIZE) != buffer)
		return "Uknown error";
#else
	if (strerror_r(errorCode, buffer, BUFFER_SIZE) != 0)
		return "Unknown error";
#endif
	return buffer;
}

bool dsPerformCheck(const char* tag, const char* file, unsigned int line, const char* function,
	bool result, const char* statementStr)
{
	if (!result)
	{
		dsLog_messagef(dsLogLevel_Error, tag, file, line, function, "%s failed with error: %s",
			statementStr,dsErrorString(errno));
	}

	return result;
}
