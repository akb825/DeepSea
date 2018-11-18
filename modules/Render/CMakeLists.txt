if (DEEPSEA_SYSTEM_MSL)
	find_package(MSL CONFIG COMPONENTS Client REQUIRED)
else()
	if (DEEPSEA_SINGLE_SHARED)
		set(MSL_SHARED OFF CACHE BOOL "")
		set(MSL_INSTALL OFF CACHE BOOL "")
	else()
		set(MSL_SHARED ${DEEPSEA_SHARED} CACHE BOOL "")
		set(MSL_INSTALL ${DEEPSEA_INSTALL} CACHE BOOL "")
	endif()
	set(mslDirectory ${DEEPSEA_MODULE_DIR}/Render/ModularShaderLanguage)
	set(MSL_BUILD_TESTS OFF CACHE BOOL "")
	set(MSL_BUILD_DOCS OFF CACHE BOOL "")
	set(MSL_BUILD_COMPILE OFF CACHE BOOL "")
	set(MSL_BUILD_TOOLS OFF CACHE BOOL "")
	set(MSL_OUTPUT_DIR ${DEEPSEA_OUTPUT_DIR} CACHE PATH "")
	set(MSL_EXPORTS_DIR ${DEEPSEA_EXPORTS_DIR} CACHE PATH "")
	set(MSL_ROOT_FOLDER ${DEEPSEA_ROOT_FOLDER}/external/MSL CACHE STRING "")
	add_subdirectory(${mslDirectory})
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CompileShaders.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/ConvertTexture.cmake)

file(GLOB_RECURSE sources src/*.c src/*.h include/*.h include/*.mslh)
ds_add_library(deepsea_render MODULE Render FILES ${sources} DEPENDS deepsea_core deepsea_math
	deepsea_geometry)
ds_target_link_libraries(deepsea_render PRIVATE msl_client)

if (DEEPSEA_GPU_PROFILING)
	set(gpuProfilingEnabled 1)
else()
	set(gpuProfilingEnabled 0)
endif()
ds_target_compile_definitions(deepsea_render PRIVATE
	DS_GPU_PROFILING_ENABLED=${gpuProfilingEnabled})

if (NOT DEEPSEA_SYSTEM_MSL)
	set(externalPrefixes MSL)
endif()

# Only need to list it as a dependency if not a shared library.
if (NOT DEEPSEA_SHARED)
	set(externalDependencies MSLClient)
endif()

ds_install_library(TARGET deepsea_render MODULE Render DEPENDS Core Math Geometry
	EXTERNAL_PREFIXES ${externalPrefixes} EXTERNAL_DEPENDS ${externalDependencies}
	CONFIG_FILES cmake/CompileShaders.cmake cmake/ConvertTexture.cmake
	CONFIG_LINES
		"include(\${CMAKE_CURRENT_LIST_DIR}/CompileShaders.cmake)"
		"include(\${CMAKE_CURRENT_LIST_DIR}/ConvertTexture.cmake)")
