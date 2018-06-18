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

# ds_create_vector_resources(container
#                            FILE file
#                            OUTPUT ARGS_OUTPUT
#                            [DEPENDENCY pattern1 [pattern2 ...]]
#                            [DEPENDENCY_RECURSE pattern1 [pattern2 ...]]
#                            [WORKING_DIR dir])
#
# Creates vector resources to be shared among vector images.
#
# container - name of a variable to hold the vector resources that will be created.
# FILE - the input json file to describe the resources
# OUTPUT - the path of the ARGS_OUTPUT.
# DEPENDENCY - list of patterns to be used as dependencies. A GLOB will be performed for each
#              pattern.
# DEPENDENCY_RECURSE - same as DEPENDENCY, except each pattern performs a GLOB_RECURSE.
# WORKING_DIR - the working directory for creating the vector resources.
function(ds_create_vector_resources container)
	if (NOT PYTHONINTERP_FOUND)
		message(FATAL_ERROR "Python not found on the path.")
	endif()
	if (NOT CUTTLEFISH)
		message(FATAL_ERROR "Program 'cuttlefish' not found on the path.")
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

	if (TARGET deepsea_vector_draw_flatbuffers)
		list(APPEND deps deepsea_vector_draw_flatbuffers)
	endif()

	set(createVectorResources ${DEEPSEA_SOURCE_DIR}/python/CreateVectorResources.py)
	add_custom_command(OUTPUT ${ARGS_OUTPUT}
		COMMAND ${PYTHON_EXECUTABLE} ARGS ${createVectorResources}
			-i ${ARGS_FILE} -o ${ARGS_OUTPUT} -c ${CUTTLEFISH} -j
		DEPENDS ${deps} ${recursiveDeps} ${ARGS_FILE} ${CUTTLEFISH} ${createVectorResources}
		COMMENT "Creating vector resources: ${ARGS_OUTPUT}")

	set(${container} ${${container}} ${ARGS_OUTPUT} PARENT_SCOPE)
endfunction()

# ds_create_vector_resources_target(target container)
#
# Adds a target to create vector resources made from previous calls to ds_create_vector_resources().
#
# target - the name of the target.
# container - the container previously passed to ds_create_vector_resources().
# All following arguments are forwarded to add_custom_target().
function(ds_create_vector_resources_target target container)
	add_custom_target(${target} DEPENDS ${${container}} ${ARGN})
endfunction()
