if (DEEPSEA_SYSTEM_MSL)
	find_package(MSL COMPONENTS Client REQUIRED)
else()
	if (DEEPSEA_SINGLE_SHARED)
		set(MSL_SHARED OFF CACHE BOOL "")
		set(MSL_INSTALL OFF CACHE BOOL "")
	else()
		set(MSL_SHARED ${DEEPSEA_SHARED} CACHE BOOL "")
		set(MSL_INSTALL ${DEEPSEA_INSTALL} CACHE BOOL "")
	endif()
	set(MSL_STATIC_RUNTIME ${DEEPSEA_STATIC_RUNTIME} CACHE BOOL "")
	set(MSL_BUILD_TESTS OFF CACHE BOOL "")
	set(MSL_BUILD_DOCS OFF CACHE BOOL "")
	set(MSL_BUILD_COMPILE OFF CACHE BOOL "")
	set(MSL_BUILD_TOOLS OFF CACHE BOOL "")
	set(MSL_INSTALL_SET_RPATH OFF CACHE BOOL "")
	set(MSL_OUTPUT_DIR ${DEEPSEA_OUTPUT_DIR} CACHE PATH "")
	set(MSL_EXPORTS_DIR ${DEEPSEA_EXPORTS_DIR} CACHE PATH "")
	set(MSL_ROOT_FOLDER ${DEEPSEA_ROOT_FOLDER}/external/MSL CACHE STRING "")
	add_subdirectory(ModularShaderLanguage)
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CompileShaders.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/ConvertTexture.cmake)

file(GLOB_RECURSE sources src/*.c src/*.h include/*.h include/*.mslh)
ds_add_library(deepsea_render MODULE Render FILES ${sources})
ds_target_link_libraries(deepsea_render PRIVATE MSL::Client)
ds_target_include_directories(deepsea_render PRIVATE src)

if (DEEPSEA_GPU_PROFILING)
	set(gpuProfilingEnabled 1)
else()
	set(gpuProfilingEnabled 0)
endif()
ds_target_compile_definitions(deepsea_render PUBLIC
	DS_GPU_PROFILING_ENABLED=${gpuProfilingEnabled})

# Only need to list it as a dependency if not a shared library.
if (NOT DEEPSEA_SHARED)
	set(externalDependencies MSLClient)
endif()

ds_install_library(TARGET deepsea_render MODULE Render DEPENDS Core Math Geometry
	EXTERNAL_DEPENDS ${externalDependencies}
	CONFIG_FILES cmake/CompileShaders.cmake cmake/ConvertTexture.cmake
	CONFIG_LINES
		"include(\${CMAKE_CURRENT_LIST_DIR}/CompileShaders.cmake)"
		"include(\${CMAKE_CURRENT_LIST_DIR}/ConvertTexture.cmake)")
