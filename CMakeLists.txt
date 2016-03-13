cmake_minimum_required(VERSION 3.0.2)
project(DeepSea)
	
# Build options
set(DEEPSEA_SHARED OFF CACHE BOOL "Build DeepSea using shared libraries.")

# Options for disabling portions of the build.
set(DEEPSEA_BUILD_TESTS ON CACHE BOOL "Build unit tests.")
set(DEEPSEA_BUILD_DOCS ON CACHE BOOL "Build documentation.")

# Misc options.
set(DEEPSEA_ROOT_FOLDER DeepSea CACHE STRING
	"Root folder for the DeepSea projects. Usefull when embedding in other projects")
	
if (DEEPSEA_BUILD_TESTS)
	find_package(GTest QUIET)
	if (NOT GTEST_FOUND)
		message("GTest not installed. Skipping tests.")
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

set(DEEPSEA_MAJOR_VERSION 0)
set(DEEPSEA_MINOR_VERSION 0)
set(DEEPSEA_PATCH_VERSION 1)
set(DEEPSEA_VERSION ${DEEPSEA_MAJOR_VERSION}.${DEEPSEA_MINOR_VERSION}.${DEEPSEA_PATCH_VERSION})

include(cmake/config.cmake)
include(cmake/install.cmake)

add_subdirectory(Core)
add_subdirectory(doc)

install_master_config()
