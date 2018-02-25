# Copyright 2018 Aaron Barany
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

find_program(FLATC flatc)
if (NOT FLATC)
	message("flatc not installed. Using pre-generated flatbuffers.")
endif()

find_file(FLATBUFFERS_INCLUDE_DIRS flatbuffers/flatbuffers.h PATHS ${DEEPSEA_SOURCE_DIR}/external)

# ds_convert_flatbuffers(container
#                        FILE file1 [file2 ...]
#                        DIRECTORY directory)
#
# Converts a list of flatbuffers into generated headers.
#
# container - name of a variable to hold the generated headers.
# FILE - the list of files to convert.
# DIRECTORY - the directory to place the flatbuffers.
function(ds_convert_flatbuffers)
	if (NOT FLATC)
		return()
	endif()

	set(oneValueArgs DIRECTORY)
	set(multiValueArgs FILE)
	cmake_parse_arguments(ARGS "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
	if (NOT ARGS_DIRECTORY)
		message(FATAL_ERROR "Required option DIRECTORY not specified.")
		return()
	endif()
	if (NOT ARGS_FILE)
		message(FATAL_ERROR "Required option FILE not specified.")
		return()
	endif()

	set(outputs)
	foreach (file ${ARGS_FILE})
		get_filename_component(filename ${file} NAME_WE)
		set(output ${ARGS_DIRECTORY}/${filename}_generated.h)
		list(APPEND outputs ${output})

		add_custom_command(OUTPUT ${output}
			MAIN_DEPENDENCY ${file}
			COMMAND ${FLATC} ARGS -c --scoped-enums ${file}
			DEPENDS ${FLATC}
			WORKING_DIRECTORY ${ARGS_DIRECTORY})
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
