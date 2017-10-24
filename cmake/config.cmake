# Copyright 2017 Aaron Barany
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

if (MSVC)
	set(commonFlags "/W3 /WX /wd4146 /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_WARNINGS /MP")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${commonFlags}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${commonFlags}")
else()
	if (DEEPSEA_SHARED AND
		(CMAKE_C_COMPILER_ID MATCHES "GNU" OR CMAKE_C_COMPILER_ID MATCHES "Clang"))
		set(otherCFlags "-fvisibility=hidden")
		set(otherCXXFlags "${otherCFlags} -fvisibility-inlines-hidden")
	else()
		set(otherCFlags)
		set(otherCXXFlags)
	endif()

	set(commonFlags "-fPIC -Wall -Werror -Wconversion -Wno-sign-conversion -fno-strict-aliasing")
	if (APPLE)
		set(commonFlags "${commonFlags} -fobjc-arc")
	endif()
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${commonFlags} ${otherCFlags}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${commonFlags} -std=c++11 ${otherCXXFlags}")
endif()

enable_testing()

function(ds_set_folder target folderName)
	if (DEEPSEA_ROOT_FOLDER AND folderName)
		set_property(TARGET ${target} PROPERTY FOLDER ${DEEPSEA_ROOT_FOLDER}/${folderName})
	elseif (NOT DEEPSEA_ROOT_FOLDER AND folderName)
		set_property(TARGET ${target} PROPERTY FOLDER ${folderName})
	else()
		set_property(TARGET ${target} PROPERTY FOLDER ${DEEPSEA_ROOT_FOLDER})
	endif()
endfunction()

function(ds_setup_filters)
	set(options)
	set(oneValueArgs SRC_DIR INCLUDE_DIR FOLDER)
	set(multiValueArgs FILES)
	cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	if (ARGS_FOLDER)
		set(prefix "${ARGS_FOLDER}/")
	else()
		set(prefix)
	endif()

	foreach (fileName ${ARGS_FILES})
		# Get the directory name. Make sure there's a trailing /.
		get_filename_component(directoryName ${fileName} DIRECTORY)
		set(directoryName ${directoryName}/)

		set(filterName)
		string(REGEX MATCH ${ARGS_SRC_DIR}/.* matchSrc ${directoryName})
		string(REGEX MATCH ${ARGS_INCLUDE_DIR}/.* matchInclude ${directoryName})

		if (matchSrc)
			string(REPLACE ${ARGS_SRC_DIR}/ "" filterName ${directoryName})
			set(filterName src/${prefix}${filterName})
		elseif (matchInclude)
			string(REPLACE ${ARGS_INCLUDE_DIR}/ "" filterName ${directoryName})
			set(filterName include/${prefix}${filterName})
		endif()

		if (filterName)
			string(REPLACE "/" "\\" filterName ${filterName})
			source_group(${filterName} FILES ${fileName})
		endif()
	endforeach()
endfunction()

function(ds_glob_library_sources result moduleName)
	if (DEEPSEA_SINGLE_SHARED)
		foreach(arg ${ARGN})
			set(args ${args} ${moduleName}/${arg})
		endforeach()
	else()
		set(args ${ARGN})
	endif()

	file(GLOB_RECURSE tempResult ${args})
	set(${result} ${tempResult} PARENT_SCOPE)
endfunction()

macro(ds_add_module moduleName)
	set(DEEPSEA_MODULES ${DEEPSEA_MODULES} ${moduleName})
	if (DEEPSEA_SINGLE_SHARED)
		include(${CMAKE_CURRENT_SOURCE_DIR}/${moduleName}/library.cmake)
	else()
		add_subdirectory(${moduleName})
	endif()
endmacro()

macro(ds_finish_modules)
	if (DEEPSEA_SINGLE_SHARED)
		add_library(deepsea ${DEEPSEA_LIB} ${DEEPSEA_ALL_SOURCES})
		if (DEEPSEA_EXTERNAL_LIBRARIES)
			target_link_libraries(deepsea ${DEEPSEA_EXTERNAL_LIBRARIES})
		endif()
		if (DEEPSEA_INCLUDE_DIRECTORIES)
			target_include_directories(deepsea ${DEEPSEA_INCLUDE_DIRECTORIES})
		endif()
		if (DEEPSEA_COMPILE_DEFINITIONS)
			target_compile_definitions(deepsea ${DEEPSEA_COMPILE_DEFINITIONS})
		endif()
		ds_set_folder(deepsea libs)

		foreach (module ${DEEPSEA_MODULES})
			add_subdirectory(${module})
		endforeach()
	endif()
endmacro()

macro(ds_add_module_library target)
	if (DEEPSEA_SINGLE_SHARED)
		add_library(${target} INTERFACE)
		target_link_libraries(${target} INTERFACE deepsea ${ARGN})
	else()
		include(${CMAKE_CURRENT_SOURCE_DIR}/library.cmake)
		target_link_libraries(${target} PUBLIC ${ARGN})
		ds_set_folder(${target} libs)
	endif()
endmacro()

macro(ds_add_library target moduleName)
	get_filename_component(mainModule ${moduleName} NAME)
	if (DEEPSEA_SINGLE_SHARED)
		set(DEEPSEA_ALL_SOURCES ${DEEPSEA_ALL_SOURCES} ${ARGN})
		set(DEEPSEA_INTERFACE_INCLUDES ${DEEPSEA_INTERFACE_INCLUDES}
			$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${moduleName}/include>
			$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/${moduleName}/include>)
		ds_setup_filters(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${moduleName}/src
			INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${moduleName}/include/DeepSea/${mainModule}
			FILES ${ARGN} FOLDER ${mainModule})
	else()
		add_library(${target} ${DEEPSEA_LIB} ${ARGN})
		ds_setup_filters(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src
			INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/DeepSea/${mainModule}
			FILES ${ARGN})
	endif()
endmacro()

macro(ds_target_link_libraries target)
	if (DEEPSEA_SINGLE_SHARED)
		set(DEEPSEA_EXTERNAL_LIBRARIES ${DEEPSEA_EXTERNAL_LIBRARIES} ${ARGN})
	else()
		target_link_libraries(${target} ${ARGN})
	endif()
endmacro()

macro(ds_target_include_directories target)
	if (DEEPSEA_SINGLE_SHARED)
		set(DEEPSEA_INCLUDE_DIRECTORIES ${DEEPSEA_INCLUDE_DIRECTORIES} ${ARGN})
	else()
		target_include_directories(${target} ${ARGN})
	endif()
endmacro()

macro(ds_target_compile_definitions target)
	if (DEEPSEA_SINGLE_SHARED)
		set(DEEPSEA_COMPILE_DEFINITIONS ${DEEPSEA_COMPILE_DEFINITIONS} ${ARGN})
	else()
		target_compile_definitions(${target} ${ARGN})
	endif()
endmacro()
