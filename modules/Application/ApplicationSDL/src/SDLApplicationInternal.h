/*
 * Copyright 2022-2026 Aaron Barany
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

#include <DeepSea/Core/Config.h>
#include <DeepSea/Application/Types.h>

#include <SDL3/SDL.h>

typedef struct dsSDLApplication
{
	dsApplication application;

	bool useMotionSensors;
	bool quit;
	bool hasFrameEvents;
	int exitCode;
	SDL_Cursor* cursors[dsCursor_Count];
	dsCursor curCursor;
	uint64_t inputTickRef;
	uint64_t inputNSRef;
	uint64_t lastFrameTicks;
} dsSDLApplication;

bool dsSDLApplication_useMotionSensors(const dsApplication* application);
bool dsSDLApplication_iterate(dsApplication* application);
bool dsSDLApplication_event(dsApplication* application, SDL_Event* sdlEvent);
