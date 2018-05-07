cmake_minimum_required(VERSION 3.1)
project(DeepSea)

# Build options
set(DEEPSEA_SHARED OFF CACHE BOOL "Build DeepSea using shared libraries.")
set(DEEPSEA_SINGLE_SHARED OFF CACHE BOOL "Build DeepSea as a single shared library for all modules.")
set(DEEPSEA_PROFILING ON CACHE BOOL "Enable profiling of code.")
set(DEEPSEA_SYSTEM_MSL OFF CACHE BOOL "Use the system installed version of MSL.")

if (DEEPSEA_SINGLE_SHARED)
	set(DEEPSEA_SHARED ON CACHE BOOL "" FORCE)
endif()

# Options for disabling portions of the build.
set(DEEPSEA_BUILD_TESTS ON CACHE BOOL "Build unit tests.")
set(DEEPSEA_BUILD_DOCS ON CACHE BOOL "Build documentation.")

set(DEEPSEA_BUILD_RENDER ON CACHE BOOL "Build rendering libraries.")
set(DEEPSEA_BUILD_RENDER_MOCK ON CACHE BOOL "Build mock rendering implementation, used by tests.")
set(DEEPSEA_BUILD_RENDER_OPENGL ON CACHE BOOL "Build OpenGL rendering implementation.")

set(DEEPSEA_BUILD_TEXT ON CACHE BOOL "Build text rendering library.")
set(DEEPSEA_BUILD_VECTOR_DRAW ON CACHE BOOL "Build vector draw library.")

set(DEEPSEA_BUILD_APPLICATION ON CACHE BOOL "Build application framework libraries.")
set(DEEPSEA_BUILD_APPLICATION_SDL ON CACHE BOOL "Build SDL application implementation.")

# OpenGL specific options.
set(DEEPSEA_GLES OFF CACHE BOOL "Use OpenGL ES rather than desktop OpenGL.")
set(DEEPSEA_TARGET_GLES_VERSION "30" CACHE STRING
	"The version of OpenGL ES to use times 10. This is currently only used for Apple platforms.")
set(DEEPSEA_PREFER_EGL OFF CACHE BOOL "Prefer EGL to the platform-specific loader.")

# Misc options.
set(DEEPSEA_OUTPUT_DIR ${CMAKE_BINARY_DIR}/output CACHE PATH "Folder for placing the build output.")
set(DEEPSEA_EXPORTS_DIR ${CMAKE_BINARY_DIR}/cmake CACHE PATH
	"Folder for placing the cmake exports while building. Useful when embedding in other projects.")
set(DEEPSEA_ROOT_FOLDER DeepSea CACHE STRING
	"Root folder for the DeepSea projects. Usefull when embedding in other projects.")
set(DEEPSEA_INSTALL ON CACHE BOOL "Allow installation for DeepSea components.")

# Call this function to get the following variables:
# DEEPSEA_SOURCE_DIR: Root source directory for DeepSea.
# DEEPSEA_MODULE_DIR: Root directory for the modules of DeepSea.
# DEEPSEA_MODULES: The list of mudles. These can be used as paths relative to DEEPSEA_MODULE_DIR.
macro(ds_get_variables)
	get_property(DEEPSEA_SOURCE_DIR GLOBAL PROPERTY DEEPSEA_SOURCE_DIR)
	get_property(DEEPSEA_MODULE_DIR GLOBAL PROPERTY DEEPSEA_MODULE_DIR)
	get_property(DEEPSEA_MODULES GLOBAL PROPERTY DEEPSEA_MODULES)
endmacro()

if (DEEPSEA_BUILD_TESTS)
	find_package(GTest QUIET)
	if (NOT GTEST_FOUND)
		message("GTest not installed. Skipping unit tests.")
	endif()
endif()

if (DEEPSEA_BUILD_DOCS)
	find_package(Doxygen QUIET)
	if (NOT DOXYGEN_FOUND)
		message("Doxygon not installed. Skipping documentation.")
	endif()
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (DEEPSEA_SHARED)
	set(DEEPSEA_LIB SHARED)
else()
	set(DEEPSEA_LIB STATIC)
endif()

if (DEEPSEA_OUTPUT_DIR)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${DEEPSEA_OUTPUT_DIR})
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${DEEPSEA_OUTPUT_DIR})
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${DEEPSEA_OUTPUT_DIR})
endif()

set(DEEPSEA_MAJOR_VERSION 0)
set(DEEPSEA_MINOR_VERSION 0)
set(DEEPSEA_PATCH_VERSION 1)
set(DEEPSEA_VERSION ${DEEPSEA_MAJOR_VERSION}.${DEEPSEA_MINOR_VERSION}.${DEEPSEA_PATCH_VERSION})

set(DEEPSEA_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(DEEPSEA_MODULE_DIR ${DEEPSEA_SOURCE_DIR}/modules)
set_property(GLOBAL PROPERTY DEEPSEA_SOURCE_DIR ${DEEPSEA_SOURCE_DIR})
set_property(GLOBAL PROPERTY DEEPSEA_MODULE_DIR ${DEEPSEA_MODULE_DIR})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${DEEPSEA_SOURCE_DIR}/cmake)
include(cmake/config.cmake)
include(cmake/install.cmake)

add_subdirectory(modules)

if (DEEPSEA_BUILD_TESTS)
	add_subdirectory(testers)
endif()

# Documentation. (populated by built libraries)
add_subdirectory(doc)
