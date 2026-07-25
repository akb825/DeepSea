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

#define SDL_MAIN_NOIMPL 1
#include <SDL3/SDL_main.h>

int SDLCALL dsSDL_RunApp(int argc, char *argv[], dsMainFunction mainFunction, void *reserved)
{
	return SDL_RunApp(argc, argv, mainFunction, reserved);
}
