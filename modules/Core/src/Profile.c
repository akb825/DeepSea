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
#include <string.h>

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
	if (!gFunctions.startFrameFunc)
		return;

	gFunctions.startFrameFunc(gUserData);
}

void dsProfile_endFrame()
{
	if (!gFunctions.endFrameFunc)
		return;

	gFunctions.endFrameFunc(gUserData);
}

void dsProfile_push(void** localData, dsProfileType type, const char* name, const char* file,
	const char* function, unsigned int line, bool dynamicName)
{
	if (!gFunctions.pushFunc)
		return;

	gFunctions.pushFunc(gUserData, localData, type, name, file, function, line, dynamicName);
}

void dsProfile_pop(dsProfileType type, const char* file, const char* function, unsigned int line)
{
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

void dsProfile_gpu(const char* surface, const char* pass, uint64_t timeNs)
{
	if (!gFunctions.gpuFunc)
		return;

	gFunctions.gpuFunc(gUserData, surface, pass, timeNs);
}
