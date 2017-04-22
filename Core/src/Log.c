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

#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Assert.h>
#include <assert.h>
#include <stdio.h>

#if DS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string.h>
#endif

static void* gUserData;
static dsLogFunction gFunction;

static const char* logLevelStrings[] =
{
	"trace",
	"debug",
	"info",
	"warning",
	"error",
	"fatal"
};

void dsLog_defaultPrint(dsLogLevel level, const char* tag, const char* file,
	unsigned int line, const char* function, const char* message)
{
	// Use standard assert here since DS_ASSERT relies on the output.
	assert(level >= dsLogLevel_Trace && level <= dsLogLevel_Fatal);
#if DS_DEBUG
	if (level < dsLogLevel_Debug)
		return;
#else
	if (level < dsLogLevel_Info)
		return;
#endif

#if DS_WINDOWS
	char buffer[DS_LOG_MAX_LENGTH];
	FILE* dest;
	int length;

	if (level < dsLogLevel_Warning)
	{
		dest = stdout;
		length = snprintf(buffer, DS_LOG_MAX_LENGTH, "%s: [%s] %s\n", logLevelStrings[level],
			tag, message);
	}
	else
	{
		dest = stderr;
		length = snprintf(buffer, DS_LOG_MAX_LENGTH, "%s(%u) : %s(): %s: [%s] %s\n", file, line,
			function, logLevelStrings[level], tag, message);
	}

	assert(length >= 0);
	if (length < 0)
		return;

	fwrite(buffer, sizeof(char), strlen(buffer), dest);
	OutputDebugStringA(buffer);
#else
	if (level < dsLogLevel_Warning)
		fprintf(stdout, "%s: [%s] %s\n", logLevelStrings[level], tag, message);
	else
	{
		fprintf(stderr, "%s:%u %s(): %s: [%s] %s\n", file, line, function, logLevelStrings[level],
			tag, message);
	}
#endif
}

void dsLog_setFunction(void* userData, dsLogFunction function)
{
	gUserData = userData;
	gFunction = function;
}

void* dsLog_getUserData(void)
{
	return gUserData;
}

dsLogFunction dsLog_getFunction(void)
{
	return gFunction;
}

void dsLog_clearFunction(void)
{
	gUserData = NULL;
	gFunction = NULL;
}

void dsLog_message(dsLogLevel level, const char* tag, const char* file,
	unsigned int line, const char* function, const char* message)
{
	DS_ASSERT(level >= dsLogLevel_Trace && level <= dsLogLevel_Fatal);
	if (gFunction)
		gFunction(gUserData, level, tag, file, line, function, message);
	else
		dsLog_defaultPrint(level, tag, file, line, function, message);
}

void dsLog_messagef(dsLogLevel level, const char* tag, const char* file,
	unsigned int line, const char* function, const char* message, ...)
{
	va_list args;
	va_start(args, message);
	dsLog_vmessagef(level, tag, file, line, function, message, args);
	va_end(args);
}

void dsLog_vmessagef(dsLogLevel level, const char* tag, const char* file,
	unsigned int line, const char* function, const char* message, va_list args)
{
	char buffer[DS_LOG_MAX_LENGTH];
	va_list argsCopy;

	va_copy(argsCopy, args);
	int length = vsnprintf(buffer, DS_LOG_MAX_LENGTH, message, argsCopy);
	DS_ASSERT(length >= 0);
	if (length >= 0)
	{
		DS_ASSERT(length <= DS_LOG_MAX_LENGTH);
		buffer[length] = 0;
		dsLog_message(level, tag, file, line, function, buffer);
	}
	va_end(argsCopy);
	va_end(args);
}
