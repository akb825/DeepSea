# Copyright 2020 Aaron Barany
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

find_program(VFC_LOCAL vfc PATHS ${DEEPSEA_PREBUILT_TOOLS_DIR}/bin NO_DEFAULT_PATH)
if (VFC_LOCAL)
	set(VFC ${VFC_LOCAL} CACHE FILEPATH "vfc vertex format converter tool")
else()
	find_program(VFC_SYSTEM vfc NO_CMAKE_FIND_ROOT_PATH)
	if (VFC_SYSTEM)
		set(VFC ${VFC_SYSTEM} CACHE FILEPATH "vfc vertex format converter tool")
	else()
		set(VFC VFC-NOTFOUND)
	endif()
endif()

# ds_create_view(container
#                           FILE file
#                           OUTPUT ARGS_OUTPUT
#                           [DEPENDS pattern1 [pattern2 ...]]
#                           [DEPENDS_RECURSE pattern1 [pattern2 ...]]
#                           [WORKING_DIRECTORY dir]
#                           [MODULE_DIRECTORIES dir1 [dir2 ... ]]
#                           [EXTENSIONS extension1 [extension2  ...]])
#
# Creates view to be loaded at runtime.
#
# container - name of a variable to hold the view that will be created.
# FILE - the input json file to describe the resources
# OUTPUT - the path of the view, typiclly with the .dssr extension.
# DEPENDS - list of patterns to be used as dependencies. A GLOB will be performed for each pattern.
# DEPENDS_RECURSE - same as DEPENDS, except each pattern performs a GLOB_RECURSE.
# WORKING_DIRECTORY - the working directory for creating the view.
# MODULE_DIRECTORIES - additional directories to use for the Python module path.
# EXTENSIONS - list of Python modules to use as extensions.
function(ds_create_view container)
	if (NOT PYTHONINTERP_FOUND)
		message(FATAL_ERROR "Python not found on the path.")
	endif()
	if (NOT CUTTLEFISH)
		message(FATAL_ERROR "Program 'cuttlefish' not found on the path.")
	endif()
	if (NOT VFC)
		message(FATAL_ERROR "Program 'vfc' not found on the path.")
	endif()

	set(oneValueArgs FILE OUTPUT WORKING_DIRECTORY)
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

	set(createView ${DEEPSEA_PYTHON_DIR}/CreateView.py)
	add_custom_command(OUTPUT ${ARGS_OUTPUT}
		COMMAND ${CMAKE_COMMAND} ARGS -E env ${moduleDirs} ${PYTHON_EXECUTABLE} ${createView}
			-i ${ARGS_FILE} -o ${ARGS_OUTPUT} ${extensions}
		DEPENDS ${deps} ${recursiveDeps} ${ARGS_FILE} ${CUTTLEFISH} ${VFC} ${createView}
		${workingDir}
		COMMENT "Creating view: ${ARGS_OUTPUT}")

	set(${container} ${${container}} ${ARGS_OUTPUT} PARENT_SCOPE)
endfunction()

# ds_create_view_target(target container)
#
# Adds a target to create view made from previous calls to ds_create_view().
#
# target - the name of the target.
# container - the container previously passed to ds_create_view().
# All following arguments are forwarded to add_custom_target().
function(ds_create_view_target target container)
	add_custom_target(${target} DEPENDS ${${container}} ${ARGN})
endfunction()
