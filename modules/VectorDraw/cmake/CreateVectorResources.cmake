# Copyright 2017-2023 Aaron Barany
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

# ds_create_vector_resources(container
#                            FILE file
#                            OUTPUT ARGS_OUTPUT
#                            [DEPENDS pattern1 [pattern2 ...]]
#                            [DEPENDS_RECURSE pattern1 [pattern2 ...]]
#                            [WORKING_DIRECTORY dir])
#
# Creates vector resources to be shared among vector images.
#
# container - name of a variable to hold the vector resources that will be created.
# FILE - the input json file to describe the resources
# OUTPUT - the path of the vector resources, typiclly with the .dsvr extension.
# DEPENDS - list of patterns to be used as dependencies. A GLOB will be performed for each pattern.
# DEPENDS_RECURSE - same as DEPENDS, except each pattern performs a GLOB_RECURSE.
# WORKING_DIRECTORY - the working directory for creating the vector resources.
function(ds_create_vector_resources container)
	if (NOT Python_FOUND)
		message(FATAL_ERROR "Python not found on the path.")
	endif()
	if (NOT CUTTLEFISH)
		message(FATAL_ERROR "Program 'cuttlefish' not found on the path.")
	endif()

	set(oneValueArgs FILE OUTPUT WORKING_DIRECTORY)
	set(multiValueArgs DEPENDS DEPENDS_RECURSE)
	cmake_parse_arguments(ARGS "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
	if (NOT ARGS_FILE)
		message(FATAL_ERROR "Required option FILE not specified.")
		return()
	endif()
	if (NOT ARGS_OUTPUT)
		message(FATAL_ERROR "Required option OUTPUT not specified.")
		return()
	endif()

	file(GLOB deps ${ARGS_DEPENDS})
	file(GLOB_RECURSE recursiveDeps ${ARGS_DEPENDS_RECURSE})
	if (ARGS_WORKING_DIRECTORY)
		set(workingDir WORKING_DIRECTORY ${ARGS_WORKING_DIRECTORY})
	else()
		set(workingDir "")
	endif()

	if (TARGET deepsea_vector_draw_flatbuffers)
		list(APPEND deps deepsea_vector_draw_flatbuffers)
	endif()

	set(createVectorResources ${DEEPSEA_PYTHON_DIR}/CreateVectorResources.py)
	set(command ${Python_EXECUTABLE} ${createVectorResources} -i ${ARGS_FILE} -o ${ARGS_OUTPUT}
		-c ${CUTTLEFISH})

	# NOTE: Output file doesn't support generator expressions, so need to manually expand it.
	if (ARGS_OUTPUT MATCHES ".*\\$<CONFIG>.*")
		foreach (config ${CMAKE_CONFIGURATION_TYPES})
			string(REPLACE "$<CONFIG>" ${config} configOutput ${ARGS_OUTPUT})

			# Need generator expression for each and every argument.
			set(configCommand)
			foreach (arg ${command})
				list(APPEND configCommand $<$<CONFIG:${config}>:${arg}>)
			endforeach()

			add_custom_command(OUTPUT ${configOutput} COMMAND ${configCommand}
				DEPENDS ${deps} ${recursiveDeps} ${ARGS_FILE} ${convertSvg}
				${workingDir}
				COMMENT "Creating ${config} vector image: ${ARGS_OUTPUT}")
		endforeach()
	else()
		add_custom_command(OUTPUT ${ARGS_OUTPUT}
			COMMAND ${command}
			DEPENDS ${deps} ${recursiveDeps} ${ARGS_FILE} ${CUTTLEFISH} ${createVectorResources}
			${workingDir}
			COMMENT "Creating vector resources: ${ARGS_OUTPUT}")
	endif()

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
