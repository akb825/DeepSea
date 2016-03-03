#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Assert.h>
#include <stdio.h>
#include <stdbool.h>

#if DS_WINDOWS
#include WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string.h>
#endif

static void* gUserData;
static dsLog_FunctionType gFunction;

static const char* logLevelStrings[] =
{
	"trace",
	"debug",
	"info",
	"warning",
	"error",
	"fatal"
};

static void defaultLog(dsLogLevel level, const char* tag, const char* file,
	unsigned int line, const char* function, const char* message)
{
#if DS_WINDOWS
	char buffer[DS_LOG_MAX_LENGTH];
	FILE* dest;
	int length;
	if (level < dsLogLevel_Warning)
	{
		dest = stdout;
		length = snprintf(buffer, DS_LOG_MAX_LENGTH, "%s: %s - %s\n", logLevelStrings[level],
			tag, message);
	}
	else
	{
		length = snprintf(buffer, DS_LOG_MAX_LENGTH, "%s(%u) : %s() %s: %s - %s\n", file, line,
			function, logLevelStrings[level], tag, message);
	}

	DS_ASSERT(length >= 0);
	if (length < 0)
		return;

	fwrite(buffer, sizeof(char), strlen(buffer), dest);
	OutputDebugStringA(buffer);
#else
	if (level < dsLogLevel_Warning)
		fprintf(stdout, "%s: %s - %s\n", logLevelStrings[level], tag, message);
	else
	{
		fprintf(stderr, "%s:%u %s() %s: %s - %s\n", file, line, function, logLevelStrings[level],
			tag, message);
	}
#endif
}

void dsLog_setFunction(void* userData, dsLog_FunctionType function)
{
	gUserData = userData;
	gFunction = function;
}

void* dsLog_getUserData()
{
	return gUserData;
}

void* dsLog_getFunction()
{
	return gFunction;
}

void dsLog_clearFunction()
{
	gUserData = NULL;
	gFunction = NULL;
}

void dsLog_message(dsLogLevel level, const char* tag, const char* file,
	unsigned int line, const char* function, const char* message)
{
	if (gFunction)
		gFunction(gUserData, level, tag, file, line, function, message);
	else
		defaultLog(level, tag, file, line, function, message);
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
		dsLog_message(level, tag, file, line, function, buffer);
	va_end(argsCopy);
	va_end(args);
}
