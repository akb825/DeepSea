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

set(DEEPSEA_PREBUILT_TOOLS_DIR ${DEEPSEA_SOURCE_DIR}/dependencies/tools)
if (IS_DIRECTORY ${DEEPSEA_PREBUILT_TOOLS_DIR} AND DEEPSEA_INSTALL)
	install(DIRECTORY ${DEEPSEA_PREBUILT_TOOLS_DIR}/ DESTINATION share/deepsea/tools
		USE_SOURCE_PERMISSIONS PATTERN "version" EXCLUDE)
endif()

set(DEEPSEA_PREBUILT_LIBS_DIR)
if (NOT DEEPSEA_NO_PREBUILT_LIBS)
	set(DEEPSEA_PREBUILT_LIBS_DIR_BASE ${DEEPSEA_SOURCE_DIR}/dependencies/libs)
	if (ANDROID)
		set(DEEPSEA_PREBUILT_LIBS_DIR ${DEEPSEA_PREBUILT_LIBS_DIR_BASE}/android-${ANDROID_ABI})
	elseif (WIN32)
		if (DEEPSEA_ARCH STREQUAL x86_64)
			set(DEEPSEA_PREBUILT_LIBS_DIR ${DEEPSEA_PREBUILT_LIBS_DIR_BASE}/win64)
		elseif (DEEPSEA_ARCH STREQUAL x86)
			set(DEEPSEA_PREBUILT_LIBS_DIR ${DEEPSEA_PREBUILT_LIBS_DIR_BASE}/win32)
		endif()
	elseif (IOS)
		set(DEEPSEA_PREBUILT_LIBS_DIR ${DEEPSEA_PREBUILT_LIBS_DIR_BASE}/ios)
	elseif (APPLE)
		if (DEEPSEA_ARCH STREQUAL x86_64 AND
				IS_DIRECTORY ${DEEPSEA_PREBUILT_LIBS_DIR_BASE}/mac-x86_64)
			set(DEEPSEA_PREBUILT_LIBS_DIR ${DEEPSEA_PREBUILT_LIBS_DIR_BASE}/mac-x86_64)
		elseif (DEEPSEA_ARCH STREQUAL arm64 AND
				IS_DIRECTORY ${DEEPSEA_PREBUILT_LIBS_DIR_BASE}/mac-arm64)
			set(DEEPSEA_PREBUILT_LIBS_DIR ${DEEPSEA_PREBUILT_LIBS_DIR_BASE}/mac-arm64)
		else()
			set(DEEPSEA_PREBUILT_LIBS_DIR ${DEEPSEA_PREBUILT_LIBS_DIR_BASE}/mac)
		endif()
	else()
		if (DEEPSEA_ARCH STREQUAL x86_64)
			set(DEEPSEA_PREBUILT_LIBS_DIR ${DEEPSEA_PREBUILT_LIBS_DIR_BASE}/linux)
		endif()
	endif()

	if (DEEPSEA_PREBUILT_LIBS_DIR AND IS_DIRECTORY ${DEEPSEA_PREBUILT_LIBS_DIR})
		set(CMAKE_FIND_ROOT_PATH ${CMAKE_FIND_ROOT_PATH} ${DEEPSEA_PREBUILT_LIBS_DIR})
		set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${DEEPSEA_PREBUILT_LIBS_DIR})

		file(GLOB prebuiltDlls ${DEEPSEA_PREBUILT_LIBS_DIR}/bin/*.dll)
		if (WIN32 AND DEEPSEA_OUTPUT_DIR)
			# Copy any DLLs for Windows.
			foreach (config ${CMAKE_CONFIGURATION_TYPES})
				file(COPY ${prebuiltDlls} DESTINATION ${DEEPSEA_OUTPUT_DIR}/${config})
			endforeach()
		endif()

		if (DEEPSEA_INSTALL)
			file(GLOB prebuiltLibs ${DEEPSEA_PREBUILT_LIBS_DIR}/lib/*.a
				${DEEPSEA_PREBUILT_LIBS_DIR}/lib/*.lib)
			file(GLOB prebuiltSharedLibs ${DEEPSEA_PREBUILT_LIBS_DIR}/lib/*.so
				${DEEPSEA_PREBUILT_LIBS_DIR}/lib/*.dylib)
			file(GLOB prebuiltDlls ${DEEPSEA_PREBUILT_LIBS_DIR}/bin/*.dll)

			# Exclude gtest libraries as they are only for unit tests.
			file(GLOB gtestLibs ${DEEPSEA_PREBUILT_LIBS_DIR}/lib/*gtest*
				${DEEPSEA_PREBUILT_LIBS_DIR}/lib/*gmock* ${DEEPSEA_PREBUILT_LIBS_DIR}/bin/*gtest*
				${DEEPSEA_PREBUILT_LIBS_DIR}/bin/*gmock*)
			list(REMOVE_ITEM prebuiltLibs ${gtestLibs})
			list(REMOVE_ITEM prebuiltSharedLibs ${gtestLibs})
			list(REMOVE_ITEM prebuiltDlls ${gtestLibs})

			# Only the static or dynamic SDL library based on what's needed.
			if (NOT ANDROID)
				if (DEEPSEA_SHARED)
					set(unusedSdlLibs ${DEEPSEA_PREBUILT_LIBS_DIR}/lib/libSDL2.a
						${DEEPSEA_PREBUILT_LIBS_DIR}/lib/SDL2-static.lib)
				else()
					set(unusedSdlLibs ${DEEPSEA_PREBUILT_LIBS_DIR}/lib/libSDL2.so
						${DEEPSEA_PREBUILT_LIBS_DIR}/lib/libSDL2.dylib
						${DEEPSEA_PREBUILT_LIBS_DIR}/lib/SDL2.lib
						${DEEPSEA_PREBUILT_LIBS_DIR}/lib/SDL2.dll)
				endif()
				list(REMOVE_ITEM prebuiltLibs ${unusedSdlLibs})
				list(REMOVE_ITEM prebuiltSharedLibs ${unusedSdlLibs})
				list(REMOVE_ITEM prebuiltDlls ${unusedSdlLibs})
			endif()

			install(FILES ${prebuiltLibs} DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT dev)
			install(PROGRAMS ${prebuiltSharedLibs} DESTINATION ${CMAKE_INSTALL_LIBDIR}
				COMPONENT dev)
			install(PROGRAMS ${prebuiltDlls} DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT dev)
		endif()
	endif()
endif()
