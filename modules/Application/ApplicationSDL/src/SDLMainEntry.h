/*
* Copyright 2026 Aaron Barany
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
#include <DeepSea/ApplicationSDL/Export.h>
#include <SDL3/SDL.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef int (SDLCALL *dsMainFunction)(int argc, char* argv[]);

// Callbacks expected by SDL, which should link once everything is brought together.
DS_APPLICATIONSDL_EXPORT SDL_AppResult SDLCALL SDL_AppIterate(void* appstate);
DS_APPLICATIONSDL_EXPORT SDL_AppResult SDLCALL SDL_AppEvent(void* appstate, SDL_Event* event);
DS_APPLICATIONSDL_EXPORT void SDLCALL SDL_AppQuit(void* appstate, SDL_AppResult result);

// Wrapped SDL functions so SDL itself only needs to be linked once.
DS_APPLICATIONSDL_EXPORT int SDLCALL dsSDL_EnterAppMainCallbacks(int argc, char* argv[],
	SDL_AppInit_func appinit, SDL_AppIterate_func appiter, SDL_AppEvent_func appevent,
	SDL_AppQuit_func appquit);
DS_APPLICATIONSDL_EXPORT int SDLCALL dsSDL_RunApp(
	int argc, char* argv[], dsMainFunction mainFunction, void* reserved);

DS_APPLICATIONSDL_EXPORT SDL_AppResult dsApplicationSDL_initResult(
	const dsApplication* application);

#ifdef __cplusplus
}
#endif
