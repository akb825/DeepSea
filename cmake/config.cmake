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

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (MSVC)
	add_compile_options(/W3 /WX /wd4146 /MP)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS)

	# Disable RTTI, but enable exceptions
	if (CMAKE_CXX_FLAGS MATCHES "/EHs-c- ")
		string(REPLACE "/EHs-c-" "/EHsc" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	elseif (NOT CMAKE_CXX_FLAGS MATCHES "/EHsc ")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
	endif()

	if (CMAKE_CXX_FLAGS MATCHES "/GR ")
		string(REPLACE "/GR" "/GR-" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	elseif (NOT CMAKE_CXX_FLAGS MATCHES "/GR- ")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR-")
	endif()
else()
	add_compile_options(-Wall -Werror -Wconversion -Wno-sign-conversion -fno-strict-aliasing)
	if (APPLE)
		add_compile_options(-fobjc-arc)
	endif()
	# Behavior for VISIBILITY_PRESET variables are inconsistent between CMake versions.
	if (DEEPSEA_SHARED)
		add_compile_options(-fvisibility=hidden)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility-inlines-hidden")
	endif()

	# Disable RTTI, but enable exceptions
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fexceptions")
endif()

enable_testing()

function(ds_start_modules)
	if (DEEPSEA_SINGLE_SHARED)
		# NOTE: Need at least one source to prevent CMake warning.
		add_library(deepsea ${DEEPSEA_LIB} "")
	endif()
endfunction()

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

macro(ds_add_module moduleName)
	get_filename_component(mainModuleName ${moduleName} NAME)
	set_property(GLOBAL APPEND PROPERTY DEEPSEA_MODULE_PATHS ${moduleName})
	set_property(GLOBAL APPEND PROPERTY DEEPSEA_MODULES ${mainModuleName})
	add_subdirectory(${moduleName})
endmacro()

macro(ds_finish_modules)
	if (DEEPSEA_SINGLE_SHARED)
		get_property(sources GLOBAL PROPERTY DEEPSEA_SOURCES)
		get_property(externalSources GLOBAL PROPERTY DEEPSEA_EXTERNAL_SOURCES)
		get_property(modules GLOBAL PROPERTY DEEPSEA_MODULES)
		get_property(externalLibraries GLOBAL PROPERTY DEEPSEA_EXTERNAL_LIBRARIES)

		if (externalSources)
			if (MSVC)
				set_source_files_properties(${externalSources} PROPERTIES COMPILE_FLAGS /w)
			else()
				set_source_files_properties(${externalSources} PROPERTIES COMPILE_FLAGS -w)
			endif()
		endif()

		foreach (module ${modules})
			get_property(filters GLOBAL PROPERTY DEEPSEA_${module}_FILTERS)
			ds_setup_filters(${filters})

			get_property(filters GLOBAL PROPERTY DEEPSEA_${module}_EXTERNAL_FILTERS)
			if (filters)
				ds_setup_filters(${filters})
			endif()
		endforeach()

		target_sources(deepsea PRIVATE ${sources} ${externalSources})
		target_link_libraries(deepsea ${externalLibraries})
		ds_set_folder(deepsea modules)
	endif()
endmacro()

macro(ds_add_library target)
	set(options)
	set(oneValueArgs MODULE)
	set(multiValueArgs FILES EXTERNAL_FILES DEPENDS)
	cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	if (NOT ARGS_MODULE)
		message(FATAL_ERROR "No module set for ds_add_library()")
	endif()

	if (DEEPSEA_SINGLE_SHARED)
		add_library(${target} INTERFACE)
		target_link_libraries(${target} INTERFACE deepsea ${ARGS_DEPENDS})

		set_property(GLOBAL APPEND PROPERTY DEEPSEA_SOURCES ${ARGS_FILES})
		set_property(GLOBAL APPEND PROPERTY DEEPSEA_EXTERNAL_SOURCES ${ARGS_EXTERNAL_FILES})

		set_source_files_properties(${ARGS_FILES} PROPERTIES COMPILE_FLAGS -w)
		target_include_directories(deepsea PUBLIC
			$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
			$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>)

		set_property(GLOBAL PROPERTY DEEPSEA_${ARGS_MODULE}_FILTERS SRC_DIR
			${CMAKE_CURRENT_SOURCE_DIR}/src INCLUDE_DIR
			${CMAKE_CURRENT_SOURCE_DIR}/include/DeepSea/${ARGS_MODULE}
			FILES ${ARGS_FILES} FOLDER ${ARGS_MODULE})
		if (ARGS_EXTERNAL_FILES)
			set_property(GLOBAL PROPERTY DEEPSEA_${ARGS_MODULE}_EXTERNAL_FILTERS SRC_DIR
				${CMAKE_CURRENT_SOURCE_DIR} FILES ${ARGS_EXTERNAL_FILES} FOLDER ${ARGS_MODULE})
		endif()
	else()
		# NOTE: This only takes affect if set in the same scope as the target was created.
		if (ARGS_EXTERNAL_FILES)
			if (MSVC)
				set_source_files_properties(${ARGS_EXTERNAL_FILES} PROPERTIES COMPILE_FLAGS /w)
			else()
				set_source_files_properties(${ARGS_EXTERNAL_FILES} PROPERTIES COMPILE_FLAGS -w)
			endif()
		endif()

		add_library(${target} ${DEEPSEA_LIB} ${ARGS_FILES} ${ARGS_EXTERNAL_FILES})
		target_link_libraries(${target} PUBLIC ${ARGS_DEPENDS})

		ds_set_folder(${target} modules)
		ds_setup_filters(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src
			INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/DeepSea/${ARGS_MODULE}
			FILES ${ARGS_FILES})
		if (ARGS_EXTERNAL_FILES)
			ds_setup_filters(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ARGS_EXTERNAL_FILES})
		endif()
	endif()
endmacro()

macro(ds_add_executable target)
	set(options WIN32 MACOSX_BUNDLE)
	set(oneValueArgs)
	set(multiValueArgs)
	cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	if (ANDROID)
		add_library(${target} SHARED ${ARGS_UNPARSED_ARGUMENTS})
	else()
		set(ARGS_FORWARDED)
		if (ARGS_WIN32)
			list(APPEND ARGS_FORWARDED WIN32)
		endif()
		if (ARGS_MACOSX_BUNDLE)
			list(APPEND ARGS_FORWARDED MACOSX_BUNDLE)
		endif()

		add_executable(${target} ${ARGS_FORWARDED} ${ARGS_UNPARSED_ARGUMENTS})
	endif()
endmacro()

macro(ds_target_link_libraries target)
	if (DEEPSEA_SINGLE_SHARED)
		set_property(GLOBAL APPEND PROPERTY DEEPSEA_EXTERNAL_LIBRARIES ${ARGN})
	else()
		target_link_libraries(${target} ${ARGN})
	endif()
endmacro()

macro(ds_target_include_directories target)
	if (DEEPSEA_SINGLE_SHARED)
		target_include_directories(deepsea ${ARGN})
	else()
		target_include_directories(${target} ${ARGN})
	endif()
endmacro()

macro(ds_target_compile_definitions target)
	if (DEEPSEA_SINGLE_SHARED)
		target_compile_definitions(deepsea ${ARGN})
	else()
		target_compile_definitions(${target} ${ARGN})
	endif()
endmacro()

macro(ds_build_assets_dir output)
	if (ANDROID)
		# Get the assets directory based on the structure defined by Android Studio.
		# Root build directory for the APK.
		string(REGEX MATCH ".*/build" _buildDir ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
		# Final assets directory based on the root build directory and target name.
		get_filename_component(_appDir ${_buildDir} DIRECTORY)
		set(${output} ${_appDir}/${DEEPSEA_ANDROID_ASSETS_DIR})
	else()
		if (CMAKE_RUNTIME_OUTPUT_DIRECTORY)
			set(${output} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
		else()
			set(${output} ${CMAKE_CURRENT_BINARY_DIR})
		endif()

		if (CMAKE_CONFIGURATION_TYPES)
			set(${output} ${${output}}/$<CONFIG>)
		endif()
	endif()
endmacro()
