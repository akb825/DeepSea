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

#include "SDLMainEntry.h"

#include "SDLApplicationInternal.h"
#include <DeepSea/Application/Application.h>

#define SDL_MAIN_USE_CALLBACKS 1
#define SDL_MAIN_NOIMPL 1
#include <SDL3/SDL_main.h>

SDL_AppResult SDLCALL SDL_AppIterate(void* appstate)
{
	dsApplication* application = (dsApplication*)appstate;
	dsSDLApplication* sdlApplication = (dsSDLApplication*)application;

	if (!dsSDLApplication_iterate(application))
		return sdlApplication->exitCode == 0 ? SDL_APP_SUCCESS : SDL_APP_FAILURE;
	return SDL_APP_CONTINUE;
}

SDL_AppResult SDLCALL SDL_AppEvent(void* appstate, SDL_Event* event)
{
	dsApplication* application = (dsApplication*)appstate;
	dsSDLApplication* sdlApplication = (dsSDLApplication*)application;

	if (!dsSDLApplication_event(application, event))
		return sdlApplication->exitCode == 0 ? SDL_APP_SUCCESS : SDL_APP_FAILURE;
	return SDL_APP_CONTINUE;
}

void SDLCALL SDL_AppQuit(void* appstate, SDL_AppResult result)
{
	DS_UNUSED(result);
	dsApplication_destroy((dsApplication*)appstate);
}

int SDLCALL dsSDL_EnterAppMainCallbacks(int argc, char* argv[], SDL_AppInit_func appinit,
	SDL_AppIterate_func appiter, SDL_AppEvent_func appevent, SDL_AppQuit_func appquit)
{
	return SDL_EnterAppMainCallbacks(argc, argv, appinit, appiter, appevent, appquit);
}

int SDLCALL dsSDL_RunApp(int argc, char* argv[], dsMainFunction mainFunction, void* reserved)
{
	return SDL_RunApp(argc, argv, mainFunction, reserved);
}

SDL_AppResult dsApplicationSDL_initResult(const dsApplication* application)
{
	const dsSDLApplication* sdlApplication = (const dsSDLApplication*)application;
	if (sdlApplication->quit)
		return sdlApplication->exitCode == 0 ? SDL_APP_SUCCESS : SDL_APP_FAILURE;
	return SDL_APP_CONTINUE;
}
