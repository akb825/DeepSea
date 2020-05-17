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

# ds_config_binary_dir(output [path])
#
# Gets the current binary directory. When multiple configurations are used with the same generated
# project (e.g. Visual Studio, Xcode), this will append the configuration name to the directory.
# Note that this uses a generator expression, so the returned path is only valid for CMake commands
# that support generator expressions.
#
# output - variable name to place the directory into
# path - optional path to append after CMAKE_CURRENT_BINARY_DIR but before the config
macro(ds_config_binary_dir output)
	set(${output} ${CMAKE_CURRENT_BINARY_DIR})
	if (${ARGC} GREATER 1)
		set(${output} ${${output}}/${ARGV1})
	endif()
	if (CMAKE_CONFIGURATION_TYPES)
		set(${output} ${${output}}/$<CONFIG>)
	endif()
endmacro()

# ds_build_assets_dir(output target)
#
# Gets the assets directory to build assets to.
#
# This may depend on the current CMake directory being processed and the current configuration.
# This will depend on whether you override the CMAKE_RUNTIME_OUTPUT_DIRECTORY and the platform.
# On Android, you can globally change the assets directory by setting the
# DEEPSEA_ANDROID_ASSETS_DIR variable. By default it will use src/main/assets.
#
# output - variable name to place the directory into.
# target - the target to get the output directory for
macro(ds_build_assets_dir output target)
	if (ANDROID)
		# Get the assets directory based on the structure defined by Android Studio.
		# Root build directory for the APK.
		string(REPLACE "\\" "/" _outputDir ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
		string(REGEX MATCH ".*/build" _buildDir ${_outputDir})
		# Final assets directory based on the root build directory and target name.
		get_filename_component(_appDir ${_buildDir} DIRECTORY)

		if (DEEPSEA_ANDROID_ASSETS_DIR)
			set(_assetsDir ${DEEPSEA_ANDROID_ASSETS_DIR})
		else()
			set(_assetsDir src/main/assets)
		endif()
		set(${output} ${_appDir}/${_assetsDir})
	else()
		if (CMAKE_RUNTIME_OUTPUT_DIRECTORY)
			set(${output} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
		else()
			set(${output} ${CMAKE_CURRENT_BINARY_DIR})
		endif()

		if (CMAKE_CONFIGURATION_TYPES)
			set(${output} ${${output}}/$<CONFIG>)
		endif()

		if (IOS)
			if (CMAKE_RUNTIME_OUTPUT_DIRECTORY)
				set(${output} ${${output}}/${target}.app)
			else()
				set(${output} ${${output}}-iphoneos/${target}.app)
			endif()
		endif()
	endif()
endmacro()

# ds_link_main_lib(target mainLib)
#
# Links to a main library, such as deepsea_application_sdl_main.
#
# Some platforms require additional linker flags to properly link the main library.
#
# target - The target to link the library to.
# mainLib - The main library to link to.
function(ds_link_main_lib target mainLib)
	if (ANDROID)
		target_link_libraries(${target} PRIVATE "-Wl,--whole-archive" ${mainLib}
			"-Wl,--no-whole-archive")
	else()
		target_link_libraries(${target} PRIVATE ${mainLib})
	endif()
endfunction()

# ds_configure_file(inputFile outputFile
#                   [CONFIG_VAR var1 [var2 ...]])
#
# Same as configure_file(), but handles different configurations set with the $<CONFIG> generator
# expression.
#
# inputFile - The input file to configure.
# outputFile - The output file to configure. If this contains $<CONFIG>, it will be processed for
#              each configuration.
# CONFIG_VAR - Variables that contain $<CONFIG> to be replaced.
# All remaining parameters will be forwarded to configure_file().
function(ds_configure_file inputFile outputFile)
	set(options COPYONLY ESCAPE_QUOTES @ONLY)
	set(oneValueArgs NEWLINE_STYLE )
	set(multiValueArgs CONFIG_VAR)
	cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
	
	set(forwardedArgs)
	foreach (arg ${options})
		if (ARGS_${arg})
			list(APPEND forwardedArgs ${arg})
		endif()
	endforeach()
	
	foreach (arg ${oneValueArgs})
		if (ARGS_${arg})
			list(APPEND forwardedArgs ${arg} ${ARGS_${arg}})
		endif()
	endforeach()
	
	if (NOT outputFile MATCHES ".*\\$<CONFIG>.*")
		configure_file(${inputFile} ${outputFile} ${forwardedArgs})
		return()
	endif()
	
	foreach (configVar ${ARGS_CONFIG_VAR})
		set(${configVar}_ORIG ${${configVar}})
	endforeach()
	
	foreach (compilerConfig ${CMAKE_CONFIGURATION_TYPES})
		string(REPLACE "$<CONFIG>" ${compilerConfig} configOutputFile ${outputFile})
		foreach (configVar ${ARGS_CONFIG_VAR})
			string(REPLACE "$<CONFIG>" ${compilerConfig} ${configVar} ${${configVar}_ORIG})
		endforeach()

		configure_file(${inputFile} ${configOutputFile} ${forwardedArgs})
	endforeach()
endfunction()
