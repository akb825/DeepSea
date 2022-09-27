# Copyright 2018-2021 Aaron Barany
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
		if (DEEPSEA_INSTALL)
			file(GLOB prebuiltLibs ${DEEPSEA_PREBUILT_LIBS_DIR}/lib/*.a
				${DEEPSEA_PREBUILT_LIBS_DIR}/lib/*.lib)
			file(GLOB prebuiltSharedLibs ${DEEPSEA_PREBUILT_LIBS_DIR}/lib/*.so
				${DEEPSEA_PREBUILT_LIBS_DIR}/lib/*.dylib)
			file(GLOB prebuiltDlls ${DEEPSEA_PREBUILT_LIBS_DIR}/bin/*.dll)
			install(FILES ${prebuiltLibs} DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT dev)
			install(PROGRAMS ${prebuiltSharedLibs} DESTINATION ${CMAKE_INSTALL_LIBDIR}
				COMPONENT dev)
			install(PROGRAMS ${prebuiltDlls} DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT dev)
		endif()
	endif()
endif()
