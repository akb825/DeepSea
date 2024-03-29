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

# Find flatc and the flatbuffer includes. Only use system flatbuffers and automatic generation if
# both are found to avoid incompatibilities.
find_program(FLATC flatc)
find_path(SYSTEM_FLATBUFFERS_INCLUDE_DIRS flatbuffers/flatbuffers.h)
if (FLATC AND SYSTEM_FLATBUFFERS_INCLUDE_DIRS)
	set(FLATBUFFERS_INCLUDE_DIRS ${SYSTEM_FLATBUFFERS_INCLUDE_DIRS})
	set(DS_BUILD_FLATBUFFERS ON)
else()
	set(FLATBUFFERS_INCLUDE_DIRS ${DEEPSEA_SOURCE_DIR}/external)
	set(DS_BUILD_FLATBUFFERS OFF)
	message("flatc not installed. Using pre-generated flatbuffers.")
endif()

# ds_convert_flatbuffers(container
#                        FILE file1 [file2 ...]
#                        DIRECTORY directory
#                        [INCLUDE dir1 [dir2 ...]]
#                        [PYTHON directory])
#
# Converts a list of flatbuffers into generated headers.
#
# container - name of a variable to hold the generated headers.
# FILE - the list of files to convert.
# DIRECTORY - the directory to place the flatbuffers.
# INCLUDE - directories to search for includes.
# PYTHON - additionally output python files to the specified directory.
function(ds_convert_flatbuffers container)
	if (NOT DS_BUILD_FLATBUFFERS)
		return()
	endif()

	set(oneValueArgs DIRECTORY PYTHON)
	set(multiValueArgs FILE DEPENDS INCLUDE)
	cmake_parse_arguments(ARGS "${optionArgs}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
	if (NOT ARGS_DIRECTORY)
		message(FATAL_ERROR "Required option DIRECTORY not specified.")
		return()
	endif()
	if (NOT ARGS_FILE)
		message(FATAL_ERROR "Required option FILE not specified.")
		return()
	endif()

	set(includeDirs)
	set(includeDepends)
	foreach (dir ${ARGS_INCLUDE})
		list(APPEND includeDirs -I ${dir})
		file(GLOB_RECURSE fbsFiles ${dir}/*.fbs)
		if (fbsFiles)
			list(APPEND includeDepends ${fbsFiles})
		endif()
	endforeach()

	set(outputs)
	foreach (file ${ARGS_FILE})
		# Touch the .fbs file on generation to avoid situations where the timestamp doesn't rebuild
		# the generated header checked into the repository but the system version of flatbuffers is
		# different and incompatible. Don't use VERSION_GREATER_EQUAL to support older CMake
		# versions.
		if (CMAKE_VERSION VERSION_GREATER 3.11.99)
			file(TOUCH_NOCREATE ${file})
		elseif (NOT WIN32)
			execute_process(COMMAND touch -c ${file})
		endif()

		get_filename_component(filename ${file} NAME_WE)
		set(output ${ARGS_DIRECTORY}/${filename}_generated.h)
		list(APPEND outputs ${output})

		set(pythonCommand)
		if (ARGS_PYTHON)
			set(pythonCommand COMMAND ${FLATC} ARGS --no-warnings -o ${ARGS_PYTHON} -p
				${includeDirs} ${file})
		endif()

		add_custom_command(OUTPUT ${output}
			MAIN_DEPENDENCY ${file}
			COMMAND ${FLATC} ARGS -c --scoped-enums --keep-prefix --no-warnings ${includeDirs}
				${file}
			${pythonCommand}
			DEPENDS ${FLATC} ${includeDepends}
			WORKING_DIRECTORY ${ARGS_DIRECTORY}
			COMMENT "Generating flat buffer: ${file}")
	endforeach()

	set(${container} ${${container}} ${outputs} PARENT_SCOPE)
endfunction()

# ds_convert_flatbuffers_target(target container)
#
# Adds a target to compile shaders made from previous calls to ds_convert_flatbuffers().
#
# target - the name of the target.
# container - the container previously passed to ds_convert_flatbuffers().
# All following arguments are forwarded to add_custom_target().
function(ds_convert_flatbuffers_target target container)
	add_custom_target(${target} DEPENDS ${${container}} ${ARGN})
endfunction()
