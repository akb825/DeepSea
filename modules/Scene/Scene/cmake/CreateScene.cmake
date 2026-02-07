# Copyright 2020-2026 Aaron Barany
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

# ds_create_scene(container
#                 FILE file
#                 OUTPUT ARGS_OUTPUT
#                 [DEPENDS pattern1 [pattern2 ...]]
#                 [DEPENDS_RECURSE pattern1 [pattern2 ...]]
#                 [WORKING_DIRECTORY dir]
#                 [INPUT_DIRECTORY dir]
#                 [MODULE_DIRECTORIES dir1 [dir2 ... ]]
#                 [EXTENSIONS extension1 [extension2  ...]])
#
# Creates scene to be loaded at runtime.
#
# container - name of a variable to hold the scene that will be created.
# FILE - the input json file to describe the resources
# OUTPUT - the path of the scene, typiclly with the .dssr extension.
# DEPENDS - list of patterns to be used as dependencies. A GLOB will be performed for each pattern.
# DEPENDS_RECURSE - same as DEPENDS, except each pattern performs a GLOB_RECURSE.
# WORKING_DIRECTORY - the working directory for creating the scene.
# INPUT_DIRECTORY - the input directory relative paths within the input file.
# MODULE_DIRECTORIES - additional directories to use for the Python module path.
# EXTENSIONS - list of Python modules to use as extensions.
function(ds_create_scene container)
	if (NOT Python_FOUND)
		message(FATAL_ERROR "Python not found on the path.")
	endif()

	set(oneValueArgs FILE OUTPUT WORKING_DIRECTORY INPUT_DIRECTORY)
	set(multiValueArgs DEFINE DEPENDS DEPENDS_RECURSE MODULE_DIRECTORIES EXTENSIONS)
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

	if (ARGS_MODULE_DIRECTORIES)
		if ($ENV{PYTHONPATH})
			set(moduleDirs "PYTHONPATH=$ENV{PYTHONPATH};${ARGS_MODULE_DIRECTORIES}")
		else()
			set(moduleDirs "PYTHONPATH=${ARGS_MODULE_DIRECTORIES}")
		endif()
	else()
		set(moduleDirs)
	endif()

	if (ARGS_EXTENSIONS)
		set(extensions -e ${ARGS_EXTENSIONS})
	else()
		set(extensions)
	endif()

	if (TARGET deepsea_scene_flatbuffers)
		list(APPEND deps deepsea_scene_flatbuffers)
	endif()

	set(createScene ${DEEPSEA_PYTHON_DIR}/CreateScene.py)
	set(buildCommand ${CMAKE_COMMAND} -E env ${moduleDirs} ${Python_EXECUTABLE} ${createScene}
		-i ${ARGS_FILE} -o ${ARGS_OUTPUT} ${extensions})
	if (ARGS_INPUT_DIRECTORY)
		list(APPEND buildCommand -d ${ARGS_INPUT_DIRECTORY})
	endif()

	# NOTE: Output file doesn't support generator expressions, so need to manually expand it.
	if (ARGS_OUTPUT MATCHES ".*\\$<CONFIG>.*")
		foreach (compilerConfig ${CMAKE_CONFIGURATION_TYPES})
			string(REPLACE "$<CONFIG>" ${compilerConfig} configOutput ${ARGS_OUTPUT})
			set(configCommand)
			foreach (command ${buildCommand})
				list(APPEND configCommand $<$<CONFIG:${compilerConfig}>:${command}>)
			endforeach()
			add_custom_command(OUTPUT ${configOutput} COMMAND ${configCommand}
				DEPENDS ${deps} ${recursiveDeps} ${ARGS_FILE} ${createScene}
				${workingDir}
				COMMENT "Creating scene: ${configOutput}")
		endforeach()
	else()
		add_custom_command(OUTPUT ${ARGS_OUTPUT} COMMAND ${buildCommand}
			DEPENDS ${deps} ${recursiveDeps} ${ARGS_FILE} ${createScene} ${workingDir}
			COMMENT "Creating scene: ${ARGS_OUTPUT}")
	endif()

	set(${container} ${${container}} ${ARGS_OUTPUT} PARENT_SCOPE)
endfunction()

# ds_create_scene_target(target container)
#
# Adds a target to create scene made from previous calls to ds_create_scene().
#
# target - the name of the target.
# container - the container previously passed to ds_create_scene().
# All following arguments are forwarded to add_custom_target().
function(ds_create_scene_target target container)
	add_custom_target(${target} DEPENDS ${${container}} ${ARGN})
endfunction()
