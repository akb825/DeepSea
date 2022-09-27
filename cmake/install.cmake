# Copyright 2017-2022 Aaron Barany
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
	set(multiValueArgs DEPENDS DEPENDS_NOLINK EXTERNAL_PREFIXES EXTERNAL_DEPENDS CONFIG_LINES
		CONFIG_FILES)
	cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	set(moduleName DeepSea${ARGS_MODULE})
	set(namespacedTarget DeepSea::${ARGS_MODULE})
	string(TOUPPER ${ARGS_MODULE} moduleUpper)

	if (DEEPSEA_SINGLE_SHARED AND NOT ARGS_STATIC)
		target_link_libraries(${ARGS_TARGET} INTERFACE deepsea)
	else()
		set_target_properties(${ARGS_TARGET} PROPERTIES
			VERSION ${DEEPSEA_VERSION}
			DEBUG_POSTFIX d)
		foreach (dependency ${ARGS_DEPENDS})
			target_link_libraries(${ARGS_TARGET} PUBLIC DeepSea::${dependency})
		endforeach()
	endif()
	set_target_properties(${ARGS_TARGET} PROPERTIES EXPORT_NAME ${ARGS_MODULE})
	add_library(DeepSea::${ARGS_MODULE} ALIAS ${ARGS_TARGET})

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
		set_property(TARGET ${includeTarget} APPEND PROPERTY INCLUDE_DIRECTORIES
			${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_BINARY_DIR}/include)
		if (DEEPSEA_SHARED)
			if (MSVC)
				if (DEEPSEA_SINGLE_SHARED)
					set(buildMacro "DS_BUILD")
				else()
					set(buildMacro "DS_${moduleUpper}_BUILD")
					set_property(TARGET ${ARGS_TARGET} APPEND PROPERTY COMPILE_DEFINITIONS
						DS_${moduleUpper}_BUILD)
				endif()
				configure_file(${DEEPSEA_SOURCE_DIR}/cmake/templates/WindowsExport.h.in
					${exportPath} @ONLY)
			else()
				configure_file(${DEEPSEA_SOURCE_DIR}/cmake/templates/UnixExport.h.in ${exportPath}
					@ONLY)
			endif()
		else()
			configure_file(${DEEPSEA_SOURCE_DIR}/cmake/templates/NoExport.h.in ${exportPath} @ONLY)
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
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
	install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
		COMPONENT dev)
	install(FILES ${exportPath} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/DeepSea/${ARGS_MODULE}
		COMPONENT dev)

	include(CMakePackageConfigHelpers)
	set(versionPath ${DEEPSEA_EXPORTS_DIR}/${moduleName}ConfigVersion.cmake)
	write_basic_package_version_file(${versionPath}
		VERSION ${DEEPSEA_VERSION}
		COMPATIBILITY SameMajorVersion)

	export(EXPORT ${moduleName}Targets FILE ${DEEPSEA_EXPORTS_DIR}/${moduleName}Targets.cmake)

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
	foreach (dependency ${ARGS_DEPENDS} ${ARGS_DEPENDS_NOLINK})
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
		"set(${moduleName}_LIBRARIES ${namespacedTarget})\n"
		"get_target_property(${moduleName}_INCLUDE_DIRS ${namespacedTarget} INTERFACE_INCLUDE_DIRECTORIES)\n"
		"${extraLines}")

	set(configPackageDir ${CMAKE_INSTALL_LIBDIR}/cmake/${moduleName})
	install(EXPORT ${moduleName}Targets NAMESPACE DeepSea:: FILE ${moduleName}Targets.cmake
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
		set_target_properties(deepsea PROPERTIES
			VERSION ${DEEPSEA_VERSION}
			DEBUG_POSTFIX d
			EXPORT_NAME DeepSea)
		set_property(TARGET deepsea APPEND PROPERTY COMPILE_DEFINITIONS DS_BUILD)
		set_property(TARGET deepsea APPEND PROPERTY INCLUDE_DIRECTORIES
			${CMAKE_CURRENT_BINARY_DIR}/include)
		add_library(DeepSea::DeepSea ALIAS deepsea)

		if (DEEPSEA_INSTALL)
			install(TARGETS deepsea EXPORT DeepSeaTargets
				LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
				ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
				RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
				INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
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

	set(configPackageDir ${CMAKE_INSTALL_LIBDIR}/cmake/DeepSea)
	configure_file(${DEEPSEA_SOURCE_DIR}/cmake/templates/DeepSeaConfig.cmake.in
		${DEEPSEA_EXPORTS_DIR}/DeepSeaConfig.cmake @ONLY)
	install(FILES ${DEEPSEA_EXPORTS_DIR}/DeepSeaConfig.cmake
		${DEEPSEA_SOURCE_DIR}/cmake/helpers.cmake ${versionPath}
		DESTINATION ${configPackageDir} COMPONENT dev)
	if (DEEPSEA_SINGLE_SHARED)
		install(EXPORT DeepSeaTargets NAMESPACE DeepSea:: FILE DeepSeaTargets.cmake
			DESTINATION ${configPackageDir})
	endif()
endfunction()
