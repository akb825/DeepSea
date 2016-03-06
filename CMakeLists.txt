cmake_minimum_required(VERSION 3.0.2)
project(DeepSea)

find_package(Doxygen QUIET)
find_package(GTest QUIET)
if (NOT DOXYGEN_FOUND)
	message("Doxygon not installed. Skipping documentation.")
endif()
if (NOT GTEST_FOUND)
	message("GTest not installed. Skipping tests.")
endif()

set(DEEPSEA_SHARED OFF CACHE BOOL "Build DeepSea using shared libraries.")

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
