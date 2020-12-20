/*
 * Copyright 2017 Aaron Barany
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

#include <DeepSea/Core/Config.h>
#include <SDL_main.h>

// Make sure the main function is visible on Android.
#if DS_ANDROID
#define DS_MAIN_EXPORT __attribute__((visibility("default")))
#else
#define DS_MAIN_EXPORT
#endif

#if DS_WINDOWS
__declspec(dllexport) uint32_t NvOptimusEnablement = 1;
__declspec(dllexport) uint32_t AmdPowerXpressRequestHighPerformance = 1;
#endif

extern int dsMain(int argc, const char** argv);

// SDL_main.h will #define main to SDL_main when it needs to be replaced.
DS_MAIN_EXPORT int main(int argc, char* argv[])
{
	return dsMain(argc, (const char**)argv);
}
