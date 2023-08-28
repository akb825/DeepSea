# Copyright 2020-2023 Aaron Barany
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

# ds_scene_resource_shader_module_config(stringVar fileName baseDir shaderDir versions...)
#
# Creates a string to embed in the scene resources configuration for the shader modules for various
# graphics API versions.
#
# stringVar - name of the variable to place theoutput.
# fileName - file name of the shader module without any directory components.
# baseDir - the base directory for the scene resources.
# shaderDir - the directory for shaders. This is assumed to be within baseDir.
# versions - list of shader versions that are used.
function(ds_scene_resource_shader_module_config stringVar fileName baseDir shaderDir)
	set(first ON)
	set(result "[")
	foreach (version ${ARGN})
		if (first)
			set(first OFF)
		else()
			set(result "${result}, ")
		endif()

		set(path ${shaderDir}/${version}/${fileName})
		set(result "${result}{\"version\": \"${version}\", \"module\": \"${path}\", \"output\": \"${path}\", \"outputRelativeDir\": \"${baseDir}\"}")
	endforeach()

	set(${stringVar} "${result}]" PARENT_SCOPE)
endfunction()

# ds_create_scene_resources(container
#                           FILE file
#                           OUTPUT ARGS_OUTPUT
#                           [DEPENDS pattern1 [pattern2 ...]]
#                           [DEPENDS_RECURSE pattern1 [pattern2 ...]]
#                           [WORKING_DIRECTORY dir]
#                           [MODULE_DIRECTORIES dir1 [dir2 ... ]]
#                           [EXTENSIONS extension1 [extension2  ...]])
#
# Creates scene resources to be loaded at runtime.
#
# container - name of a variable to hold the scene resources that will be created.
# FILE - the input json file to describe the resources
# OUTPUT - the path of the scene resources, typiclly with the .dssr extension.
# DEPENDS - list of patterns to be used as dependencies. A GLOB will be performed for each pattern.
# DEPENDS_RECURSE - same as DEPENDS, except each pattern performs a GLOB_RECURSE.
# WORKING_DIRECTORY - the working directory for creating the scene resources.
# MODULE_DIRECTORIES - additional directories to use for the Python module path.
# EXTENSIONS - list of Python modules to use as extensions.
function(ds_create_scene_resources container)
	if (NOT Python_FOUND)
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

	set(createSceneResources ${DEEPSEA_PYTHON_DIR}/CreateSceneResources.py)
	set(buildCommand ${CMAKE_COMMAND} -E env ${moduleDirs} ${Python_EXECUTABLE}
		${createSceneResources} -i ${ARGS_FILE} -o ${ARGS_OUTPUT} -c ${CUTTLEFISH} -v ${VFC}
		${extensions})

	# NOTE: Output file doesn't support generator expressions, so need to manually expand it.
	if (ARGS_OUTPUT MATCHES ".*\\$<CONFIG>.*")
		foreach (compilerConfig ${CMAKE_CONFIGURATION_TYPES})
			string(REPLACE "$<CONFIG>" ${compilerConfig} configOutput ${ARGS_OUTPUT})
			set(configCommand)
			foreach (command ${buildCommand})
				list(APPEND configCommand $<$<CONFIG:${compilerConfig}>:${command}>)
			endforeach()
			add_custom_command(OUTPUT ${configOutput} COMMAND ${configCommand}
				DEPENDS ${deps} ${recursiveDeps} ${ARGS_FILE} ${CUTTLEFISH} ${VFC}
					${createSceneResources}
				${workingDir}
				COMMENT "Creating scene resources: ${configOutput}")
		endforeach()
	else()
		add_custom_command(OUTPUT ${ARGS_OUTPUT} COMMAND ${buildCommand}
			DEPENDS ${deps} ${recursiveDeps} ${ARGS_FILE} ${CUTTLEFISH} ${VFC}
				${createSceneResources}
			${workingDir}
			COMMENT "Creating scene resources: ${ARGS_OUTPUT}")
	endif()

	set(${container} ${${container}} ${ARGS_OUTPUT} PARENT_SCOPE)
endfunction()

# ds_create_scene_resources_target(target container)
#
# Adds a target to create scene resources made from previous calls to ds_create_scene_resources().
#
# target - the name of the target.
# container - the container previously passed to ds_create_scene_resources().
# All following arguments are forwarded to add_custom_target().
function(ds_create_scene_resources_target target container)
	add_custom_target(${target} DEPENDS ${${container}} ${ARGN})
endfunction()
