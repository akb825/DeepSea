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

find_package(PythonInterp QUIET)

# ds_convert_svg(container
#                FILE file
#                OUTPUT ARGS_OUTPUT
#                [DEPENDENCY pattern1 [pattern2 ...]]
#                [DEPENDENCY_RECURSE pattern1 [pattern2 ...]]
#                [WORKING_DIR dir])
#
# Converts an SVG into a vector image.
#
# container - name of a variable to hold the vector image that will be created.
# FILE - the input SVG file
# OUTPUT - the path of the ARGS_OUTPUT.
# DEPENDENCY - list of patterns to be used as dependencies. A GLOB will be performed for each
#              pattern.
# DEPENDENCY_RECURSE - same as DEPENDENCY, except each pattern performs a GLOB_RECURSE.
# WORKING_DIR - the working directory for creating the vector image.
function(ds_convert_svg container)
	if (NOT PYTHONINTERP_FOUND)
		message(FATAL_ERROR "Python not found on the path.")
	endif()

	set(oneValueArgs FILE OUTPUT WORKING_DIR)
	set(multiValueArgs DEFINE DEPENDENCY DEPENDENCY_RECURSE)
	cmake_parse_arguments(ARGS "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
	if (NOT ARGS_FILE)
		message(FATAL_ERROR "Required option FILE not specified.")
		return()
	endif()
	if (NOT ARGS_OUTPUT)
		message(FATAL_ERROR "Required option OUTPUT not specified.")
		return()
	endif()

	file(GLOB deps ${ARGS_DEPENDENCY})
	file(GLOB_RECURSE recursiveDeps ${ARGS_DEPENDENCY_RECURSE})
	if (ARGS_WORKING_DIR)
		set(workingDir WORKING_DIR ${ARGS_WORKING_DIR})
	else()
		set(workingDir "")
	endif()

	set(convertSvg ${DEEPSEA_SOURCE_DIR}/python/ConvertSVG.py)
	add_custom_command(OUTPUT ${ARGS_OUTPUT}
		COMMAND ${PYTHON_EXECUTABLE} ARGS ${convertSvg}
			-i ${ARGS_FILE} -o ${ARGS_OUTPUT}
		DEPENDS ${deps} ${recursiveDeps} ${ARGS_FILE} ${convertSvg}
		COMMENT "Creating vector image: ${ARGS_OUTPUT}")

	set(${container} ${${container}} ${ARGS_OUTPUT} PARENT_SCOPE)
endfunction()

# ds_convert_svg_target(target container)
#
# Adds a target to create vector images made from previous calls to ds_convert_svg().
#
# target - the name of the target.
# container - the container previously passed to ds_convert_svg().
# All following arguments are forwarded to add_custom_target().
function(ds_convert_svg_target target container)
	add_custom_target(${target} DEPENDS ${${container}} ${ARGN})
endfunction()
