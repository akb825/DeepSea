/*
 * Copyright 2018 Aaron Barany
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
#include <DeepSea/Core/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Function for opening a dynamic library and loading symbols.
 */

/**
 * @brief String for the library prefix for the current system.
 */
#if DS_WINDOWS
#define DS_LIBRARY_PREFIX ""
#else
#define DS_LIBRARY_PREFIX "lib"
#endif

/**
 * @brief String for the library extension (including .) for the current system.
 */
#if DS_WINDOWS
#define DS_LIBRARY_EXTENSION ".dll"
#elif DS_APPLE
#define DS_LIBRARY_EXTENSION ".dylib"
#else
#define DS_LIBRARY_EXTENSION ".so"
#endif

/**
 * @brief Macro to get the standard library name.
 * @param name The name of the library. This should be a string literal.
 * @return The mangled library name.
 */
#define DS_LIBRARY_NAME(name) ( DS_LIBRARY_PREFIX name DS_LIBRARY_EXTENSION )

/**
 * @brief Opens a dynamic library.
 * @remark Rather than setting errno, this will set the error field of library with the error string
 *     on failure.
 * @remark Even if this fails, you should still call dsDynamicLib_close(). Some systems require
 *     cleanup for the error string, and failing to do so will
 * @param[out] library The library to load into.
 * @param path The path or name of the library to load.
 * @return False if the library couldn't be loaded.
 */
bool dsDynamicLib_open(dsDynamicLib* library, const char* path);

/**
 * @brief Loads a symbol from a dynamic library.
 * @remark rather than setting errno, this will set the error field of library with the error string
 *     on failure.
 * @param library The library to load the symbol from.
 * @param name The name of the symbol to load.
 * @return The loaded symbol or NULL if it couldn't be loaded.
 */
void* dsDynamicLib_loadSymbol(dsDynamicLib* library, const char* name);

/**
 * @brief Closes a dynamic library.
 * @remark This should be called even if dsDynamicLib_load() fails.
 * @param library The library to close.
 * @return False if the library couldn't be loaded.
 */
void dsDynamicLib_close(dsDynamicLib* library);

#ifdef __cplusplus
}
#endif
