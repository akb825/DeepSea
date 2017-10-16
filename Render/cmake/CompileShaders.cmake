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

find_program(MSLC mslc)

if (DEEPSEA_BUILD_RENDER_OPENGL)
	include(${DEEPSEA_SOURCE_DIR}/Render/RenderOpenGL/msl-config/ConfigPaths.cmake)
endif()

# ds_compile_shaders(container
#                    FILE file1 [file2 ...]
#                    OUTPUT output
#                    CONFIG config1 [config2 ...]
#                    [OUTPUT_DIR outputDir]
#                    [INCLUDE include1 [include2 ...]]
#                    [DEFINE define1[=value] [define2[=value] ...]]
#                    [DEPENDENCY pattern1 [pattern2 ...]]
#                    [DEPENDENCY_RECURSE pattern1 [pattern2 ...]]
#                    [WARN_NONE]
#                    [WARN_ERROR]
#                    [STRIP_SYMBOLS]
#                    [OPTIMIZE]
#                    [WORKING_DIR dir])
#
# Compiles a list of shaders into a single shader module.
#
# container - name of a variable to hold the shaders that will be compiled.
# FILE - list of files to compile.
# OUTPUT - the path of the output.
# CONFIG - list of configurations to compile with. It is expected each of them will have a variable
#          declared with the path to the configuration file. The compiled module will be placed in
#          a subdirectory with the same name as the config.
# OUTPUT_DIR - the directory of the output. If specified, the config name will be appended.
# INCLUDE - list of include paths for compilation.
# DEFINE - list of defines to apply for compilation.
# DEPENDENCY - list of patterns to be used as dependencies. A GLOB will be performed for each
#              pattern.
# DEPENDENCY_RECURSE - same as DEPENDENCY, except each pattern performs a GLOB_RECURSE.
# WARN_NONE - if specified, disables all warnings.
# WARN_ERROR - if specified, treats warnings as errors. 
# STRIP_SYMBOLS - if specified, strips symbols.
# OPTIMIZE - if specified, enables optimizations.
# WORKING_DIR - the working directory for running the shader compiler.
function(ds_compile_shaders container)
	if (NOT MSLC)
		message(FATAL_ERROR "Program 'mslc' not found on the path.")
	endif()

	set(options WARN_NONE WARN_ERROR STRIP_SYMBOLS OPTIMIZE)
	set(oneValueArgs OUTPUT OUTPUT_DIR WORKING_DIR)
	set(multiValueArgs FILE CONFIG INCLUDE DEFINE DEPENDENCY DEPENDENCY_RECURSE)
	cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
	if (NOT ARGS_OUTPUT)
		message(FATAL_ERROR "Required option OUTPUT not specified.")
		return()
	endif()
	if (NOT ARGS_FILE)
		message(FATAL_ERROR "Required option FILE not specified.")
		return()
	endif()
	if (NOT ARGS_CONFIG)
		message(FATAL_ERROR "Required option CONFIG not specified.")
		return()
	endif()

	file(GLOB deps ${ARGS_DEPENDENCY})
	file(GLOB_RECURSE recursiveDeps ${ARGS_DEPENDENCY_RECURSE})
	if (ARGS_WORKING_DIR)
		set(workingDir WORKING_DIR ${ARGS_WORKING_DIR})
	else()
		set(workingDir "")
	endif()

	set(extraArgs "")
	if (ARGS_WARN_NONE)
		list(APPEND extraArgs -w)
	endif()
	if (ARGS_WARN_ERROR)
		list(APPEND extraArgs -W)
	endif()
	if (ARGS_STRIP_SYMBOLS)
		list(APPEND extraArgs -s)
	endif()
	if (ARGS_OPTIMIZE)
		list(APPEND extraArgs -O)
	endif()

	set(outputs ${${container}})
	foreach (config ${ARGS_CONFIG})
		if (NOT ${config})
			message(FATAL_ERROR "No path for shader configuration ${config} found.")
			return()
		endif()

		set(commandLineArgs ${ARGS_FILE} -c ${${config}})
		if (ARGS_OUTPUT_DIR)
			set(output ${ARGS_OUTPUT_DIR}/${config})
			set(outputCommand COMMAND ${CMAKE_COMMAND} ARGS -E make_directory ${output})
			set(output ${output}/${ARGS_OUTPUT})
		else()
			set(outputCommand "")
			set(output ${ARGS_OUTPUT})
		endif()
		list(APPEND commandLineArgs -o ${output})
		list(APPEND outputs ${output})

		foreach (inc ${ARGS_INCLUDE})
			list(APPEND commandLineArgs -I ${inc})
		endforeach()

		foreach (define ${ARGS_DEFINE})
			list(APPEND commandLineArgs -D ${define})
		endforeach()

		add_custom_command(OUTPUT ${output}
			${outputCommand}
			COMMAND ${MSLC} ARGS ${commandLineArgs} ${extraArgs}
			DEPENDS ${deps} ${recursiveDeps} ${ARGS_FILE}
			COMMENT "Building ${config} shader: ${output}")
	endforeach()

	set(${container} ${${container}} ${outputs} PARENT_SCOPE)
endfunction()

# ds_compile_shaders_target(target container)
#
# Adds a target to compile shaders made from previous calls to ds_compile_shaders().
#
# target - the name of the target.
# container - the container previously passed to ds_compile_shaders().
function(ds_compile_shaders_target target container)
	add_custom_target(${target} DEPENDS ${${container}})
endfunction()
