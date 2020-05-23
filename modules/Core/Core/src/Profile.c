/*
 * Copyright 2016-2018 Aaron Barany
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

#include <DeepSea/Core/Profile.h>

#include <DeepSea/Core/Thread/ThreadStorage.h>
#include <DeepSea/Core/Debug.h>
#include <DeepSea/Core/Log.h>
#include <string.h>
#include <stdlib.h>

#if DS_DEBUG
typedef struct ScopeInfo
{
	const char* file;
	const char* function;
	uint32_t line;
	dsProfileType type;
} ScopeInfo;

#define MAX_PROFILE_DEPTH 128
#define PROFILE_LOG_TAG "profile"

static const char* gScopeNames[] =
{
	"Function",
	"Scope",
	"Wait",
	"Lock"
};

static DS_THREAD_LOCAL ScopeInfo gThreadScopes[MAX_PROFILE_DEPTH];
static DS_THREAD_LOCAL uint32_t gCurThreadScope;

static void printCurrentScopes(const char* file, unsigned int line, const char* function)
{
	dsLog_message(dsLogLevel_Fatal, PROFILE_LOG_TAG, file, line, function, "Current scopes:");
	for (uint32_t i = 0; i < gCurThreadScope; ++i)
	{
		const ScopeInfo* scope = gThreadScopes + i;
		dsLog_message(dsLogLevel_Fatal, PROFILE_LOG_TAG, scope->file, scope->line, scope->function,
			gScopeNames[scope->type]);
	}
}

#endif

static void* gUserData;
static dsProfileFunctions gFunctions;

void dsProfile_setFunctions(void* userData, const dsProfileFunctions* functions)
{
	if (!functions)
		dsProfile_clearFunctions();

	gUserData = userData;
	gFunctions = *functions;
}

void* dsProfile_getUserData(void)
{
	return gUserData;
}

const dsProfileFunctions* dsProfile_getFunctions(void)
{
	return &gFunctions;
}

void dsProfile_clearFunctions(void)
{
	gUserData = NULL;
	memset(&gFunctions, 0, sizeof(gFunctions));
}

void dsProfile_registerThread(const char* name)
{
	if (!gFunctions.registerThreadFunc)
		return;

	gFunctions.registerThreadFunc(gUserData, name);
}

void dsProfile_startFrame(void)
{
#if DS_DEBUG
	if (gCurThreadScope > 0)
	{
		DS_LOG_FATAL(PROFILE_LOG_TAG, "Start frame must not be inside another profile scope.");
		printCurrentScopes(__FILE__, __LINE__, __FUNCTION__);
		DS_DEBUG_BREAK();
		abort();
	}
#endif

	if (!gFunctions.startFrameFunc)
		return;

	gFunctions.startFrameFunc(gUserData);
}

void dsProfile_endFrame()
{
#if DS_DEBUG
	if (gCurThreadScope > 0)
	{
		DS_LOG_FATAL(PROFILE_LOG_TAG, "End frame must not be inside another profile scope.");
		printCurrentScopes(__FILE__, __LINE__, __FUNCTION__);
		DS_DEBUG_BREAK();
		abort();
	}
#endif

	if (!gFunctions.endFrameFunc)
		return;

	gFunctions.endFrameFunc(gUserData);
}

void dsProfile_push(void** localData, dsProfileType type, const char* name, const char* file,
	const char* function, unsigned int line, bool dynamicName)
{
#if DS_DEBUG
	if (gCurThreadScope >= MAX_PROFILE_DEPTH)
	{
		dsLog_messagef(dsLogLevel_Fatal, PROFILE_LOG_TAG, file, line, function,
			"Profile depth exceeds max of %u.", MAX_PROFILE_DEPTH);
		printCurrentScopes(file, line, function);
		DS_DEBUG_BREAK();
		abort();
	}

	ScopeInfo* scope = gThreadScopes + gCurThreadScope++;
	scope->file = file;
	scope->function = function;
	scope->line = line;
	scope->type = type;
#endif

	if (!gFunctions.pushFunc)
		return;

	gFunctions.pushFunc(gUserData, localData, type, name, file, function, line, dynamicName);
}

void dsProfile_pop(dsProfileType type, const char* file, const char* function, unsigned int line)
{
#if DS_DEBUG
	if (gCurThreadScope == 0)
	{
		dsLog_messagef(dsLogLevel_Fatal, PROFILE_LOG_TAG, file, line, function,
			"Profile pop with no corresponding push.");
		DS_DEBUG_BREAK();
		abort();
	}

	const ScopeInfo* scope = gThreadScopes + gCurThreadScope - 1;
	if (scope->type != type)
	{
		dsLog_messagef(dsLogLevel_Fatal, PROFILE_LOG_TAG, file, line, function,
			"Scope of type %s doesn't match previous scope.", gScopeNames[type]);
		printCurrentScopes(file, line, function);
		DS_DEBUG_BREAK();
		abort();
	}

	if (scope->type == dsProfileType_Function && strcmp(function, scope->function) != 0)
	{
		dsLog_message(dsLogLevel_Fatal, PROFILE_LOG_TAG, file, line, function,
			"Function pop outside of the previous function push.");
		printCurrentScopes(file, line, function);
		DS_DEBUG_BREAK();
		abort();
	}
	--gCurThreadScope;
#endif

	if (!gFunctions.popFunc)
		return;

	gFunctions.popFunc(gUserData, type, file, function, line);
}

void dsProfile_stat(void** localData, const char* category, const char* name, double value,
	const char* file, const char* function, unsigned int line, bool dynamicName)
{
	if (!gFunctions.statFunc)
		return;

	gFunctions.statFunc(gUserData, localData, category, name, value, file, function, line,
		dynamicName);
}

void dsProfile_gpu(const char* category, const char* name, uint64_t timeNs)
{
	if (!gFunctions.gpuFunc)
		return;

	gFunctions.gpuFunc(gUserData, category, name, timeNs);
}
