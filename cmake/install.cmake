function(install_library)
	set(options)
	set(oneValueArgs TARGET MODULE)
	set(multiValueArgs DEPENDENCIES)
	cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	set(moduleName DeepSea${ARGS_MODULE})
	string(TOUPPER ${ARGS_MODULE} moduleUpper)

	set_property(TARGET ${ARGS_TARGET} PROPERTY VERSION ${DEEPSEA_VERSION})
	set_property(TARGET ${ARGS_TARGET} PROPERTY SOVERSION ${DEEPSEA_MAJOR_VERSION})
	set_property(TARGET PROPERTY INTERFACE_${moduleName}_MAJOR_VERSION ${DEEPSEA_MAJOR_VERSION})
	set_property(TARGET PROPERTY INTERFACE_${moduleName}_MINOR_VERSION ${DEEPSEA_MINOR_VERSION})
	set_property(TARGET PROPERTY INTERFACE_${moduleName}_PATCH_VERSION ${DEEPSEA_PATCH_VERSION})
	set_property(TARGET APPEND PROPERTY COMPATIBLE_VERSION_STRING ${moduleName}_MAJOR_VERSION)

	set(exportPath ${CMAKE_CURRENT_BINARY_DIR}/include/DeepSea/${ARGS_MODULE}/Export.h)
	set_property(TARGET ${ARGS_TARGET} APPEND PROPERTY INCLUDE_DIRECTORIES
		${CMAKE_CURRENT_BINARY_DIR}/include)
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
	set(versionPath ${CMAKE_CURRENT_BINARY_DIR}/DeepSea/${moduleName}Version.cmake)
	write_basic_package_version_file(${versionPath}
		VERSION ${DEEPSEA_VERSION}
		COMPATIBILITY AnyNewerVersion)

	export(EXPORT ${moduleName}Targets
		FILE ${CMAKE_CURRENT_BINARY_DIR}/DeepSea/${moduleName}Targets.cmake)

	set(dependencies "include(CMakeFindDependencyMacro)")
	foreach (dependency ${ARGS_DEPENDENCIES})
		set(dependencies "${dependencies}\nfind_dependency(DeepSea${dependency} ${DEEPSEA_VERSION})")
	endforeach()

	set(configPath ${CMAKE_CURRENT_BINARY_DIR}/DeepSea/${moduleName}Config.cmake)
	file(WRITE ${configPath}
		"${dependencies}\ninclude(\${CMAKE_CURRENT_LIST_DIR}/${moduleName}Targets.cmake")

	set(configPackageDir lib/cmake/DeepSea)
	install(EXPORT ${moduleName}Targets FILE ${moduleName}Targets.cmake
		DESTINATION ${configPackageDir})
	install(FILES ${configPath} ${versionPath} DESTINATION ${configPackageDir} COMPONENT dev)
endfunction()

function(install_master_config)
	include(CMakePackageConfigHelpers)
	set(versionPath ${CMAKE_CURRENT_BINARY_DIR}/DeepSea/DeepSeaVersion.cmake)
	write_basic_package_version_file(${versionPath}
		VERSION ${DEEPSEA_VERSION}
		COMPATIBILITY AnyNewerVersion)

	install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/DeepSeaConfig.cmake ${versionPath}
		DESTINATION lib/cmake/DeepSea COMPONENT dev)
endfunction()
