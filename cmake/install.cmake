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

function(ds_install_library)
	# This function is called from a file included into the main CMakeLists.txt from a macro, so setting
	# variables in the parent scope will be for the main CMakeLists.txt.
	set(options STATIC)
	set(oneValueArgs TARGET MODULE)
	set(multiValueArgs DEPENDS EXTERNAL_PREFIXES EXTERNAL_DEPENDS CONFIG_LINES CONFIG_FILES)
	cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	set(moduleName DeepSea${ARGS_MODULE})
	string(TOUPPER ${ARGS_MODULE} moduleUpper)

	if (NOT DEEPSEA_SINGLE_SHARED)
		set_property(TARGET ${ARGS_TARGET} PROPERTY VERSION ${DEEPSEA_VERSION})
		set_property(TARGET ${ARGS_TARGET} PROPERTY SOVERSION ${DEEPSEA_MAJOR_VERSION})
		set_property(TARGET ${ARGS_TARGET} APPEND PROPERTY COMPATIBLE_VERSION_STRING
			${moduleName}_MAJOR_VERSION)
		set_property(TARGET ${ARGS_TARGET} PROPERTY DEBUG_POSTFIX d)
	endif()

	set_property(TARGET ${ARGS_TARGET} PROPERTY INTERFACE_${moduleName}_MAJOR_VERSION
		${DEEPSEA_MAJOR_VERSION})
	set_property(TARGET ${ARGS_TARGET} PROPERTY INTERFACE_${moduleName}_MINOR_VERSION
		${DEEPSEA_MINOR_VERSION})
	set_property(TARGET ${ARGS_TARGET} PROPERTY INTERFACE_${moduleName}_PATCH_VERSION
		${DEEPSEA_PATCH_VERSION})

	if (NOT ARGS_STATIC)
		set(interfaceIncludes
			$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
			$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>)
		if (DEEPSEA_SINGLE_SHARED)
			set(includeTarget deepsea)
		else()
			set(includeTarget ${ARGS_TARGET})
		endif()
		set_property(TARGET ${includeTarget} APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
			${interfaceIncludes})

		set(exportPath ${CMAKE_CURRENT_BINARY_DIR}/include/DeepSea/${ARGS_MODULE}/Export.h)
		if (NOT DEEPSEA_SINGLE_SHARED)
			set_property(TARGET ${ARGS_TARGET} APPEND PROPERTY INCLUDE_DIRECTORIES
				${CMAKE_CURRENT_BINARY_DIR}/include ${interfaceIncludes})
		endif()
		if (DEEPSEA_SHARED)
			if (MSVC)
				if (DEEPSEA_SINGLE_SHARED)
					set(buildMacro "DS_BUILD")
				else()
					set(buildMacro "DS_${moduleUpper}_BUILD")
					set_property(TARGET ${ARGS_TARGET} APPEND PROPERTY COMPILE_DEFINITIONS
						DS_${moduleUpper}_BUILD)
				endif()
				file(WRITE ${exportPath}
					"#pragma once\n\n"
					"#ifdef ${buildMacro}\n"
					"#define DS_${moduleUpper}_EXPORT __declspec(dllexport)\n"
					"#else\n"
					"#define DS_${moduleUpper}_EXPORT __declspec(dllimport)\n"
					"#endif\n")
			elseif (CMAKE_C_COMPILER_ID MATCHES "GNU" OR CMAKE_C_COMPILER_ID MATCHES "Clang")
				file(WRITE ${exportPath}
					"#pragma once\n\n"
					"#define DS_${moduleUpper}_EXPORT __attribute__((visibility(\"default\")))\n")
			else()
				file(WRITE ${exportPath}
					"#pragma once\n\n"
					"#define DS_${moduleUpper}_EXPORT\n")
			endif()
		else()
			file(WRITE ${exportPath}
				"#pragma once\n\n"
				"#define DS_${moduleUpper}_EXPORT\n")
		endif()
	endif()

	if (NOT DEEPSEA_INSTALL)
		return()
	endif()

	# Make sure that pre-built libraries are re-mapped to the insstall directories.
	if (DEEPSEA_PREBUILT_LIBS_DIR)
		get_target_property(libraries ${ARGS_TARGET} INTERFACE_LINK_LIBRARIES)
		set(finalLibraries)
		foreach (library ${libraries})
			if (library MATCHES "${DEEPSEA_PREBUILT_LIBS_DIR}/.*")
				string(REGEX REPLACE "${DEEPSEA_PREBUILT_LIBS_DIR}/(.*)" "\\1" subpath ${library})
				list(APPEND finalLibraries $<BUILD_INTERFACE:${library}>
					$<INSTALL_INTERFACE:\${_IMPORT_PREFIX}/${subpath}>)
			elseif (library)
				list(APPEND finalLibraries ${library})
			endif()
		endforeach()
		set_target_properties(${ARGS_TARGET} PROPERTIES INTERFACE_LINK_LIBRARIES
			"${finalLibraries}")
	endif()

	install(TARGETS ${ARGS_TARGET} EXPORT ${moduleName}Targets
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
		RUNTIME DESTINATION bin
		INCLUDES DESTINATION include)
	install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include COMPONENT dev)
	install(FILES ${exportPath} DESTINATION include/DeepSea/${ARGS_MODULE} COMPONENT dev)

	include(CMakePackageConfigHelpers)
	set(versionPath ${DEEPSEA_EXPORTS_DIR}/${moduleName}ConfigVersion.cmake)
	write_basic_package_version_file(${versionPath}
		VERSION ${DEEPSEA_VERSION}
		COMPATIBILITY SameMajorVersion)

	export(EXPORT ${moduleName}Targets
		FILE ${DEEPSEA_EXPORTS_DIR}/${moduleName}Targets.cmake)

	if (ARGS_EXTERNAL_PREFIXES)
		set(externalPrefixes "get_filename_component(currentDir \${CMAKE_CURRENT_LIST_FILE} DIRECTORY)\n")
		set(externalPrefixes "${externalPrefixes}get_filename_component(parentDir \${currentDir} DIRECTORY)\n")
		foreach (prefix ${ARGS_EXTERNAL_PREFIXES})
			set(externalPrefixes "${externalPrefixes}list(APPEND CMAKE_PREFIX_PATH \${parentDir}/${prefix})\n")
		endforeach()
	endif()

	if (NOT ARGS_STATIC AND DEEPSEA_SINGLE_SHARED)
		set(DEEPSEA_EXTERNAL_PREFIXES "${DEEPSEA_EXTERNAL_PREFIXES}${externalPrefixes}" PARENT_SCOPE)
		unset(externalPrefixes)
	endif()

	set(dependencies "include(CMakeFindDependencyMacro)\n")
	if (NOT ARGS_STATIC AND DEEPSEA_SINGLE_SHARED)
		set(dependencies "${dependencies}find_dependency(DeepSea ${DEEPSEA_VERSION} EXACT)\n")
	endif()
	foreach (dependency ${ARGS_DEPENDS})
		set(dependencies "${dependencies}find_dependency(DeepSea${dependency} ${DEEPSEA_VERSION} EXACT)\n")
	endforeach()

	foreach (dependency ${ARGS_EXTERNAL_DEPENDS})
		if (NOT ARGS_STATIC AND DEEPSEA_SINGLE_SHARED)
			set(DEEPSEA_EXTERNAL_DEPENDS "${DEEPSEA_EXTERNAL_DEPENDS}find_dependency(${dependency})\n")
		else()
			set(dependencies "${dependencies}find_dependency(${dependency})\n")
		endif()
	endforeach()

	set(extraLines)
	foreach(line ${ARGS_CONFIG_LINES})
		set(extraLines "${extraLines}${line}\n")
	endforeach()

	set(configPath ${DEEPSEA_EXPORTS_DIR}/${moduleName}Config.cmake)
	file(WRITE ${configPath}
		"${externalPrefixes}"
		"${dependencies}"
		"include(\${CMAKE_CURRENT_LIST_DIR}/${moduleName}Targets.cmake)\n"
		"set(${moduleName}_LIBRARIES ${ARGS_TARGET})\n"
		"get_target_property(${moduleName}_INCLUDE_DIRS ${ARGS_TARGET} INTERFACE_INCLUDE_DIRECTORIES)\n"
		"${extraLines}")

	if (WIN32)
		set(configPackageDir ${moduleName}/cmake)
	else()
		set(configPackageDir lib/cmake/${moduleName})
	endif()
	install(EXPORT ${moduleName}Targets FILE ${moduleName}Targets.cmake
		DESTINATION ${configPackageDir})
	install(FILES ${configPath} ${versionPath} ${ARGS_CONFIG_FILES} DESTINATION ${configPackageDir}
		COMPONENT dev)
endfunction()

function(ds_install_master_config)
	include(CMakePackageConfigHelpers)
	set(versionPath ${DEEPSEA_EXPORTS_DIR}/DeepSeaConfigVersion.cmake)
	write_basic_package_version_file(${versionPath}
		VERSION ${DEEPSEA_VERSION}
		COMPATIBILITY SameMajorVersion)

	if (DEEPSEA_SINGLE_SHARED)
		set_property(TARGET deepsea PROPERTY VERSION ${DEEPSEA_VERSION})
		set_property(TARGET deepsea PROPERTY SOVERSION ${DEEPSEA_MAJOR_VERSION})
		set_property(TARGET deepsea PROPERTY INTERFACE_${moduleName}_MAJOR_VERSION
			${DEEPSEA_MAJOR_VERSION})
		set_property(TARGET deepsea PROPERTY INTERFACE_${moduleName}_MINOR_VERSION
			${DEEPSEA_MINOR_VERSION})
		set_property(TARGET deepsea PROPERTY INTERFACE_${moduleName}_PATCH_VERSION
			${DEEPSEA_PATCH_VERSION})
		set_property(TARGET deepsea APPEND PROPERTY COMPATIBLE_VERSION_STRING
			${moduleName}_MAJOR_VERSION)
		set_property(TARGET deepsea PROPERTY DEBUG_POSTFIX d)
		set_property(TARGET deepsea APPEND PROPERTY COMPILE_DEFINITIONS DS_BUILD)
		set_property(TARGET deepsea APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
			${DEEPSEA_INTERFACE_INCLUDES})
		set_property(TARGET deepsea APPEND PROPERTY INCLUDE_DIRECTORIES
			${CMAKE_CURRENT_BINARY_DIR}/include ${DEEPSEA_INTERFACE_INCLUDES})

		if (DEEPSEA_INSTALL)
			install(TARGETS deepsea EXPORT DeepSeaTargets
				LIBRARY DESTINATION lib
				ARCHIVE DESTINATION lib
				RUNTIME DESTINATION bin
				INCLUDES DESTINATION include)
			export(EXPORT DeepSeaTargets FILE ${DEEPSEA_EXPORTS_DIR}/DeepSeaTargets.cmake)

			set(componentCheck)
			set(singleSharedConfig
"${DEEPSEA_EXTERNAL_PREFIXES}\
${DEEPSEA_EXTERNAL_DEPENDS}\
include(\${CMAKE_CURRENT_LIST_DIR}/DeepSeaTargets.cmake)")
		endif()
	else()
		set(componentCheck
"if (NOT DeepSea_FIND_COMPONENTS)\n\
	set(DeepSea_NOT_FOUND_MESSAGE \"The DeepSea package requires at least one component\")\n\
	set(DeepSea_FOUND False)\n\
	return()\n\
endif()")
		set(singleSharedConfig)
	endif()

	if (NOT DEEPSEA_INSTALL)
		return()
	endif()

	if (WIN32)
		set(configPackageDir DeepSea/cmake)
	else()
		set(configPackageDir lib/cmake/DeepSea)
	endif()
	configure_file(${DEEPSEA_SOURCE_DIR}/cmake/DeepSeaConfig.cmake.in
		${DEEPSEA_EXPORTS_DIR}/DeepSeaConfig.cmake @ONLY)
	install(FILES ${DEEPSEA_EXPORTS_DIR}/DeepSeaConfig.cmake
		${DEEPSEA_SOURCE_DIR}/cmake/helpers.cmake ${versionPath}
		DESTINATION ${configPackageDir} COMPONENT dev)
	if (DEEPSEA_SINGLE_SHARED)
		install(EXPORT DeepSeaTargets FILE DeepSeaTargets.cmake
			DESTINATION ${configPackageDir})
	endif()
endfunction()
