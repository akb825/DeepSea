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

#include <DeepSea/Core/DynamicLib.h>

#if DS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

static void getErrorString(dsDynamicLib* library)
{
#if DS_WINDOWS
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&library->error,
		0, NULL);
	library->errorAllocated = true;
#else
	library->error = dlerror();
	library->errorAllocated = false;
#endif
}

static void freeErrorString(dsDynamicLib* library)
{
#if DS_WINDOWS
	if (library->errorAllocated)
		LocalFree((void*)library->error);
#endif

	library->error = NULL;
	library->errorAllocated = false;
}

bool dsDynamicLib_open(dsDynamicLib* library, const char* path)
{
	if (!library || !path)
	{
		if (library)
		{
			library->library = NULL;
			library->error = "no library path provided";
			library->errorAllocated = false;
		}
		return false;
	}

#if DS_WINDOWS
	library->library = LoadLibrary(path);
#else
	library->library = dlopen(path, RTLD_NOW);
#endif

	if (!library->library)
	{
		getErrorString(library);
		return false;
	}

	library->error = NULL;
	library->errorAllocated = false;
	return true;
}

void* dsDynamicLib_loadSymbol(dsDynamicLib* library, const char* name)
{
	if (!library)
		return NULL;

	freeErrorString(library);
	if (!name)
	{
		library->error = "no symbol name provided";
		library->errorAllocated = false;
		return NULL;
	}

	if (!library->library)
	{
		library->error = "library not opened";
		library->errorAllocated = false;
		return NULL;
	}

	void* symbol;
#if DS_WINDOWS
	symbol = GetProcAddress(library->library, name);
#else
	symbol = dlsym(library->library, name);
#endif

	if (!symbol)
		getErrorString(library);

	return symbol;
}

void dsDynamicLib_close(dsDynamicLib* library)
{
	if (!library)
		return;

	freeErrorString(library);
	if (library->library)
	{
#if DS_WINDOWS
		FreeLibrary(library->library);
#else
		dlclose(library->library);
#endif
		library->library = NULL;
	}
}
