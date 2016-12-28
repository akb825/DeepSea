file(GLOB_RECURSE sources src/*.c src/*.h include/*.h)
add_library(deepsea_render ${DEEPSEA_LIB} ${sources})

set(MSL_SHARED ${DEEPSEA_SHARED} CACHE BOOL "")
set(MSL_BUILD_TESTS OFF CACHE BOOL "")
set(MSL_BUILD_DOCS OFF CACHE BOOL "")
set(MSL_BUILD_COMPILE OFF CACHE BOOL "")
set(MSL_BUILD_TOOLS OFF CACHE BOOL "")
set(MSL_EXPORTS_DIR ${DEEPSEA_EXPORTS_DIR} CACHE PATH "")
set(MSL_ROOT_FOLDER ${DEEPSEA_ROOT_FOLDER}/external/MSL CACHE STRING "")
add_subdirectory(ModularShaderLanguage)

target_link_libraries(deepsea_render PUBLIC
	deepsea_core
	deepsea_math
	deepsea_geometry
	msl_client)
	
ds_set_folder(deepsea_render libs)
ds_setup_filters(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src
	INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/DeepSea/Render
	FILES ${sources})

ds_install_library(TARGET deepsea_render MODULE Render DEPENDENCIES Core Math)
set(DEEPSEA_DOC_PROJECTS ${DEEPSEA_DOC_PROJECTS} Render PARENT_SCOPE)
