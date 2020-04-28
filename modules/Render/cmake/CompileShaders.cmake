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

find_program(MSLC_LOCAL mslc PATHS ${DEEPSEA_PREBUILT_TOOLS_DIR}/bin NO_DEFAULT_PATH)
if (MSLC_LOCAL)
	set(MSLC ${MSLC_LOCAL} CACHE FILEPATH "mslc shader compiler tool")
else()
	find_program(MSLC_SYSTEM mslc NO_CMAKE_FIND_ROOT_PATH)
	if (MSLC_SYSTEM)
		set(MSLC ${MSLC_SYSTEM} CACHE FILEPATH "mslc shader compiler tool")
	else()
		set(MSLC MSLC-NOTFOUND)
	endif()
endif()

# ds_compile_shaders(container
#                    FILE file1 [file2 ...]
#                    OUTPUT output
#                    CONFIG config1 [config2 ...]
#                    [OUTPUT_DIR outputDir]
#                    [INCLUDE include1 [include2 ...]]
#                    [DEFINE define1[=value] [define2[=value] ...]]
#                    [DEPENDS pattern1 [pattern2 ...]]
#                    [DEPENDS_RECURSE pattern1 [pattern2 ...]]
#                    [OPTIMIZE n]
#                    [WARN_NONE]
#                    [WARN_ERROR]
#                    [WORKING_DIRECTORY dir])
#
# Compiles a list of shaders into a single shader module.
#
# container - name of a variable to hold the shaders that will be compiled.
# FILE - list of files to compile.
# OUTPUT - the path of the output. If OUTPUT_DIR is specified, this should only be the filename.
# CONFIG - list of configurations to compile with. It is expected each of them will have a variable
#          declared with the path to the configuration file. The compiled module will be placed in
#          a subdirectory with the same name as the config.
# OUTPUT_DIR - the directory of the output. If specified, the config name will be appended.
# INCLUDE - list of include paths for compilation.
# DEFINE - list of defines to apply for compilation.
# DEPENDS - list of patterns to be used as dependencies. A GLOB will be performed for each
#              pattern.
# DEPENDS_RECURSE - same as DEPENDS, except each pattern performs a GLOB_RECURSE.
# OPTIMIZE - overrides the optimization level for Release builds. Values can be 0 (disable
#            optimizations),  1 (simple optimizations), and 2 (full optimizations). Defaults to 2
#            if not specified. This has no effect for Debug builds.
# WARN_NONE - if specified, disables all warnings.
# WARN_ERROR - if specified, treats warnings as errors. 
# WORKING_DIRECTORY - the working directory for running the shader compiler.
function(ds_compile_shaders container)
	if (NOT MSLC)
		message(FATAL_ERROR "Program 'mslc' not found on the path.")
	endif()

	set(options WARN_NONE WARN_ERROR)
	set(oneValueArgs OUTPUT OUTPUT_DIR WORKING_DIRECTORY OPTIMIZE)
	set(multiValueArgs FILE CONFIG INCLUDE DEFINE DEPENDS DEPENDS_RECURSE)
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
	if (ARGS_UNPARSED_ARGUMENTS)
		message(FATAL_ERROR "Unknown arguments: ${ARGS_UNPARSED_ARGUMENTS}")
	endif()

	file(GLOB deps ${ARGS_DEPENDS})
	file(GLOB_RECURSE recursiveDeps ${ARGS_DEPENDS_RECURSE})
	if (ARGS_WORKING_DIRECTORY)
		set(workingDir WORKING_DIRECTORY ${ARGS_WORKING_DIRECTORY})
	else()
		set(workingDir)
	endif()

	set(extraArgs)
	if (ARGS_WARN_NONE)
		list(APPEND extraArgs -w)
	endif()
	if (ARGS_WARN_ERROR)
		list(APPEND extraArgs -W)
	endif()
	if (NOT ARGS_OPTIMIZE)
		set(ARGS_OPTIMIZE 2)
	endif()
	list(APPEND extraArgs $<$<NOT:$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>>:-s>)
	list(APPEND extraArgs $<$<NOT:$<CONFIG:Debug>>:-O> $<$<NOT:$<CONFIG:Debug>>:${ARGS_OPTIMIZE}>)

	if (CMAKE_CONFIGURATION_TYPES)
		set(compilerConfigs ${CMAKE_CONFIGURATION_TYPES})
	else()
		set(compilerConfigs "")
	endif()

	set(outputs)
	foreach (config ${ARGS_CONFIG})
		get_property(configPath GLOBAL PROPERTY ${config})
		if (NOT configPath)
			message(FATAL_ERROR "No path for shader configuration ${config} found.")
			return()
		endif()

		set(commandLineArgs ${ARGS_FILE} -c ${configPath})
		if (ARGS_OUTPUT_DIR)
			set(output ${ARGS_OUTPUT_DIR}/${config})
			set(outputCommand ${CMAKE_COMMAND} -E make_directory ${output})
			set(output ${output}/${ARGS_OUTPUT})
		else()
			set(outputCommand)
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

		set(compileCommand ${MSLC} ${commandLineArgs} ${extraArgs})
		set(dependsArgs DEPENDS ${deps} ${recursiveDeps} ${ARGS_FILE} ${configPath} ${MSLC}
			${workingDir})

		# NOTE: Output file doesn't support generator expressions, so need to manually expand it.
		if (output MATCHES ".*\\$<CONFIG>.*")
			foreach (compilerConfig ${compilerConfigs})
				string(REPLACE "$<CONFIG>" ${compilerConfig} configOutput ${output})

				# Need generator expression for each and every argument.
				if (outputCommand)
					set(configOutputCommand COMMAND)
					foreach (command ${outputCommand})
						list(APPEND configOutputCommand $<$<CONFIG:${compilerConfig}>:${command}>)
					endforeach()
				else()
					set(configOutputCommand)
				endif()

				set(configCompileCommand COMMAND)
				foreach (command ${compileCommand})
					list(APPEND configCompileCommand $<$<CONFIG:${compilerConfig}>:${command}>)
				endforeach()

				add_custom_command(OUTPUT ${configOutput} ${configOutputCommand}
					${configCompileCommand} ${dependsArgs}
					COMMENT "Building ${config} shader: ${configOutput}")
			endforeach()
		else()
			if (outputCommand)
				set(outputCommand COMMAND ${outputCommand})
			endif()
			add_custom_command(OUTPUT ${output} ${outputCommand} COMMAND ${compileCommand}
				${dependsArgs} COMMENT "Building ${config} shader: ${output}")
		endif()
	endforeach()

	set(${container} ${${container}} ${outputs} PARENT_SCOPE)
endfunction()

# ds_compile_shaders_target(target container)
#
# Adds a target to compile shaders made from previous calls to ds_compile_shaders().
#
# target - the name of the target.
# container - the container previously passed to ds_compile_shaders().
# All following arguments are forwarded to add_custom_target().
function(ds_compile_shaders_target target container)
	add_custom_target(${target} DEPENDS ${${container}} ${ARGN})
endfunction()
