function(ds_install_library)
	set(options)
	set(oneValueArgs TARGET MODULE)
	set(multiValueArgs DEPENDENCIES EXTERNAL_PREFIXES EXTERNAL_DEPENDENCIES)
	cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	set(moduleName DeepSea${ARGS_MODULE})
	string(TOUPPER ${ARGS_MODULE} moduleUpper)

	set_property(TARGET ${ARGS_TARGET} PROPERTY VERSION ${DEEPSEA_VERSION})
	set_property(TARGET ${ARGS_TARGET} PROPERTY SOVERSION ${DEEPSEA_MAJOR_VERSION})
	set_property(TARGET ${ARGS_TARGET} PROPERTY INTERFACE_${moduleName}_MAJOR_VERSION
		${DEEPSEA_MAJOR_VERSION})
	set_property(TARGET ${ARGS_TARGET} PROPERTY INTERFACE_${moduleName}_MINOR_VERSION
		${DEEPSEA_MINOR_VERSION})
	set_property(TARGET ${ARGS_TARGET} PROPERTY INTERFACE_${moduleName}_PATCH_VERSION
		${DEEPSEA_PATCH_VERSION})
	set_property(TARGET ${ARGS_TARGET} APPEND PROPERTY COMPATIBLE_VERSION_STRING
		${moduleName}_MAJOR_VERSION)
	set_property(TARGET ${ARGS_TARGET} PROPERTY DEBUG_POSTFIX d)

	set(interfaceIncludes
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>)
	set_property(TARGET ${ARGS_TARGET} APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
		${interfaceIncludes})

	set(exportPath ${CMAKE_CURRENT_BINARY_DIR}/include/DeepSea/${ARGS_MODULE}/Export.h)
	set_property(TARGET ${ARGS_TARGET} APPEND PROPERTY INCLUDE_DIRECTORIES
		${CMAKE_CURRENT_BINARY_DIR}/include ${interfaceIncludes})
	if (DEEPSEA_SHARED)
		if (MSVC)
			set_property(TARGET ${ARGS_TARGET} APPEND PROPERTY COMPILE_DEFINITIONS
				DS_${moduleUpper}_BUILD)
			file(WRITE ${exportPath}
				"#pragma once\n\n"
				"#ifdef DS_${moduleUpper}_BUILD\n"
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

	set(dependencies "include(CMakeFindDependencyMacro)\n")
	foreach (dependency ${ARGS_DEPENDENCIES})
		set(dependencies "${dependencies}find_dependency(DeepSea${dependency} ${DEEPSEA_VERSION} EXACT)\n")
	endforeach()
	foreach (dependency ${ARGS_EXTERNAL_DEPENDENCIES})
		set(dependencies "${dependencies}find_dependency(${dependency})\n")
	endforeach()

	set(configPath ${DEEPSEA_EXPORTS_DIR}/${moduleName}Config.cmake)
	file(WRITE ${configPath}
		"${externalPrefixes}"
		"${dependencies}"
		"include(\${CMAKE_CURRENT_LIST_DIR}/${moduleName}Targets.cmake)\n"
		"set(DeepSea${ARGS_MODULE}_LIBRARIES ${ARGS_TARGET})\n"
		"get_target_property(DeepSea${ARGS_MODULE}_INCLUDE_DIRS ${ARGS_TARGET} INTERFACE_INCLUDE_DIRECTORIES)\n")

	set(configPackageDir lib/cmake/DeepSea)
	install(EXPORT ${moduleName}Targets FILE ${moduleName}Targets.cmake
		DESTINATION ${configPackageDir})
	install(FILES ${configPath} ${versionPath} DESTINATION ${configPackageDir} COMPONENT dev)
endfunction()

function(ds_install_master_config)
	include(CMakePackageConfigHelpers)
	set(versionPath ${DEEPSEA_EXPORTS_DIR}/DeepSeaConfigVersion.cmake)
	write_basic_package_version_file(${versionPath}
		VERSION ${DEEPSEA_VERSION}
		COMPATIBILITY SameMajorVersion)

	file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/cmake/DeepSeaConfig.cmake
		DESTINATION ${DEEPSEA_EXPORTS_DIR})
	install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/DeepSeaConfig.cmake ${versionPath}
		DESTINATION lib/cmake/DeepSea COMPONENT dev)
endfunction()
