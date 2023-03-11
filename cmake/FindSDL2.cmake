# Copyright 2018-2022 Aaron Barany
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

find_path(SDL2_INCLUDE_DIRS SDL.h PATH_SUFFIXES include/SDL2)

if (WIN32)
	find_library(SDL2_STATIC_LIBRARY SDL2-static)
	find_library(SDL2_DYNAMIC_LIBRARY SDL2)
else()
	find_library(SDL2_STATIC_LIBRARY libSDL2.a)
	find_library(SDL2_DYNAMIC_LIBRARY libSDL2${CMAKE_SHARED_LIBRARY_SUFFIX})
endif()

# Search for static first when static linking, otherwise dynamic first. Always prefer dynamic if
# not using the prebuilt lib.
if (SDL2_DYNAMIC_LIBRARY AND (DEEPSEA_SHARED OR NOT SDL2_STATIC_LIBRARY
		OR NOT DEEPSEA_PREBUILT_LIBS_DIR
		OR NOT SDL2_DYNAMIC_LIBRARY MATCHES "${DEEPSEA_PREBUILT_LIBS_DIR}"))
	set(SDL2_LIBRARIES ${SDL2_DYNAMIC_LIBRARY})
	set(SDL2_SHARED ON)
else()
	set(SDL2_LIBRARIES ${SDL2_STATIC_LIBRARY})
	set(SDL2_SHARED OFF)
endif()

find_library(SDL2MAIN_LIBRARIES SDL2main)

if (NOT SDL2_SHARED)
	find_package(Threads)
	list(APPEND SDL2_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})

	if (APPLE)
		if (IOS)
			list(APPEND SDL2_LIBRARIES "-framework CoreMotion" "-framework CoreBluetooth"
				"-framework CoreGraphics")
		else()
			list(APPEND SDL2_LIBRARIES "-framework Cocoa" "-framework Carbon"
				"-framework AppKit" "-framework ForceFeedback")
		endif()

		list(APPEND SDL2_LIBRARIES "-framework CoreVideo" "-framework GameController"
			"-framework CoreHaptics" "-framework IOKit" iconv)
	elseif (WIN32)
		list(APPEND SDL2_LIBRARIES winmm imm32 version uuid setupapi)
	endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2 REQUIRED_VARS SDL2_LIBRARIES SDL2MAIN_LIBRARIES
	SDL2_INCLUDE_DIRS)
