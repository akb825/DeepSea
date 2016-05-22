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

#include <DeepSea/Core/Profile.h>

static void* gUserData;
static dsProfileFrameFunction gStartFrameFunc;
static dsProfileFrameFunction gEndFrameFunc;
static dsProfilePushFunction gPushFunc;
static dsProfilePopFunction gPopFunc;
static dsProfileStatFunction gStatFunc;

bool dsProfile_setFunctions(void* userData, dsProfileFrameFunction startFrameFunc,
	dsProfileFrameFunction endFrameFunc, dsProfilePushFunction pushFunc,
	dsProfilePopFunction popFunc, dsProfileStatFunction statFunc)
{
	if (!startFrameFunc || !endFrameFunc || !pushFunc || !popFunc || !statFunc)
		return false;

	gUserData = userData;
	gStartFrameFunc = startFrameFunc;
	gEndFrameFunc = endFrameFunc;
	gPushFunc = pushFunc;
	gPopFunc = popFunc;
	gStatFunc = statFunc;
	return true;
}

void* dsProfile_getUserData()
{
	return gUserData;
}

dsProfileFrameFunction dsProfile_getStartFrameFunction()
{
	return gStartFrameFunc;
}

dsProfileFrameFunction dsProfile_getEndFrameFunction()
{
	return gEndFrameFunc;
}

dsProfilePushFunction dsProfile_getPushFunction()
{
	return gPushFunc;
}

dsProfilePopFunction dsProfile_getPopFunction()
{
	return gPopFunc;
}

dsProfileStatFunction dsProfile_getStatFunction()
{
	return gStatFunc;
}

void dsProfile_clearFunctions()
{
	gUserData = NULL;
	gStartFrameFunc = NULL;
	gEndFrameFunc = NULL;
	gPushFunc = NULL;
	gPopFunc = NULL;
	gStatFunc = NULL;
}

bool dsProfile_enabled()
{
	return gStartFrameFunc != NULL;
}

void dsProfile_startFrame(const char* file, const char* function, unsigned int line)
{
	if (!gStartFrameFunc)
		return;

	gStartFrameFunc(gUserData, file, function, line);
}

void dsProfile_endFrame(const char* file, const char* function, unsigned int line)
{
	if (!gEndFrameFunc)
		return;

	gEndFrameFunc(gUserData, file, function, line);
}

void dsProfile_push(dsProfileType type, const char* name, const char* file, const char* function,
	unsigned int line)
{
	if (!gPushFunc)
		return;

	gPushFunc(gUserData, type, name, file, function, line);
}

void dsProfile_pop(dsProfileType type, const char* file, const char* function, unsigned int line)
{
	if (!gPopFunc)
		return;

	gPopFunc(gUserData, type, file, function, line);
}

void dsProfile_stat(const char* category, const char* name, double value,
	const char* file, const char* function, unsigned int line)
{
	if (!gStatFunc)
		return;

	gStatFunc(gUserData, category, name, value, file, function, line);
}
