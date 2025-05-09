if (WIN32)
	# Needs 3.15 for CMAKE_MSVC_RUNTIME_LIBRARY.
	cmake_minimum_required(VERSION 3.15)
else()
	cmake_minimum_required(VERSION 3.12)
endif()
if (POLICY CMP0144)
	cmake_policy(SET CMP0144 NEW)
endif()
project(DeepSea)

# Build options
# Use if since BUILD_SHARED_LIBS defaults to unset.
if (BUILD_SHARED_LIBS)
	set(DEEPSEA_SHARED_DEFAULT ON)
else()
	set(DEEPSEA_SHARED_DEFAULT OFF)
endif()
set(DEEPSEA_SHARED ${DEEPSEA_SHARED_DEFAULT} CACHE BOOL "Build DeepSea using shared libraries.")
set(DEEPSEA_SINGLE_SHARED OFF CACHE BOOL "Build DeepSea as a single shared library for all modules.")
set(DEEPSEA_STATIC_RUNTIME OFF CACHE BOOL "Use static runtime library on Windows.")
set(DEEPSEA_PROFILING ON CACHE BOOL "Enable profiling of code.")
set(DEEPSEA_GPU_PROFILING OFF CACHE BOOL "Enable profiling of the GPU.")
set(DEEPSEA_SYSTEM_MSL OFF CACHE BOOL "Use the system installed version of MSL.")

if (DEEPSEA_SINGLE_SHARED)
	set(DEEPSEA_SHARED ON CACHE BOOL "" FORCE)
endif()

# Options for disabling portions of the build.
set(DEEPSEA_BUILD_TESTS ON CACHE BOOL "Build unit tests.")
set(DEEPSEA_BUILD_DOCS ON CACHE BOOL "Build documentation.")

set(DEEPSEA_BUILD_EASY_PROFILER ON CACHE BOOL "Build the easy_profiler integration for profiling.")
set(DEEPSEA_BUILD_ZIP ON CACHE BOOL "Build with support for loading .zip files.")

set(DEEPSEA_BUILD_RENDER ON CACHE BOOL "Build rendering libraries.")
set(DEEPSEA_BUILD_RENDER_MOCK ON CACHE BOOL "Build mock rendering implementation, used by tests.")
set(DEEPSEA_BUILD_RENDER_OPENGL ON CACHE BOOL "Build OpenGL rendering implementation.")
set(DEEPSEA_BUILD_RENDER_VULKAN ON CACHE BOOL "Build Vulkan rendering implementation.")
set(DEEPSEA_BUILD_RENDER_METAL ON CACHE BOOL "Build Metal rendering implementation.")

set(DEEPSEA_BUILD_TEXT ON CACHE BOOL "Build text rendering library.")
set(DEEPSEA_BUILD_VECTOR_DRAW ON CACHE BOOL "Build vector draw library.")

set(DEEPSEA_BUILD_PARTICLE ON CACHE BOOL "Build particle library.")

set(DEEPSEA_BUILD_ANIMATION ON CACHE BOOL "Build animation library.")

set(DEEPSEA_BUILD_SCENE ON CACHE BOOL "Build scene library.")

set(DEEPSEA_BUILD_PHYSICS ON CACHE BOOL "Build physics library.")

set(DEEPSEA_BUILD_APPLICATION ON CACHE BOOL "Build application framework libraries.")
set(DEEPSEA_BUILD_APPLICATION_SDL ON CACHE BOOL "Build SDL application implementation.")

# OpenGL specific options.
set(DEEPSEA_GLES OFF CACHE BOOL "Use OpenGL ES rather than desktop OpenGL.")
set(DEEPSEA_TARGET_GLES_VERSION "30" CACHE STRING
	"The version of OpenGL ES to use times 10. This is currently only used for Apple platforms.")

# Misc options.
set(DEEPSEA_OUTPUT_DIR ${CMAKE_BINARY_DIR}/output CACHE PATH "Folder for placing the build output.")
set(DEEPSEA_EXPORTS_DIR ${CMAKE_BINARY_DIR}/cmake CACHE PATH
	"Folder for placing the cmake exports while building. Useful when embedding in other projects.")
set(DEEPSEA_ROOT_FOLDER DeepSea CACHE STRING
	"Root folder for the DeepSea projects. Usefull when embedding in other projects.")
set(DEEPSEA_INSTALL ON CACHE BOOL "Allow installation for DeepSea components.")
set(DEEPSEA_INSTALL_SET_RPATH ON CACHE BOOL "Set rpath for library and tool on installation.")
set(DEEPSEA_ANDROID_ASSETS_DIR src/main/assets CACHE PATH
	"Folder relative to project app directory to place assets for Android.")
set(DEEPSEA_NO_PREBUILT_LIBS OFF CACHE BOOL "Don't use any pre-built library dependencies.")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (DEEPSEA_SHARED)
	set(DEEPSEA_LIB SHARED)
else()
	set(DEEPSEA_LIB STATIC)
endif()

if (DEEPSEA_OUTPUT_DIR AND NOT ANDROID)
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

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${DEEPSEA_SOURCE_DIR}/cmake)
include(cmake/config.cmake)
include(cmake/helpers.cmake)
include(cmake/install.cmake)
include(cmake/prebuilt.cmake)
include(cmake/ConvertFlatbuffers.cmake)

if (APPLE AND NOT IOS)
	if (NOT CMAKE_OSX_DEPLOYMENT_TARGET)
		if (DEEPSEA_ARCH STREQUAL arm64)
			# Default to 11.0 for arm64 for more Metal features.
			set(CMAKE_OSX_DEPLOYMENT_TARGET 11.0 CACHE STRING "Minimum macOS deployment version." FORCE)
		else()
			set(CMAKE_OSX_DEPLOYMENT_TARGET 10.13 CACHE STRING "Minimum macOS deployment version." FORCE)
		endif()
	endif()

	if (DEEPSEA_ARCH STREQUAL multi AND CMAKE_OSX_DEPLOYMENT_TARGET VERSION_LESS 11.0)
		# Default to 11.0 for arm64 in universal binary builds for more Metal features.
		# Requires Xcode generator.
		set(CMAKE_XCODE_ATTRIBUTE_MACOSX_DEPLOYMENT_TARGET[arch=arm64] 11.0 CACHE STRING
			"Minimum macOS deployment version for arm64." FORCE)
	endif()
endif()

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

find_package(Python COMPONENTS Interpreter QUIET)
set(DEEPSEA_PYTHON_DIR ${DEEPSEA_SOURCE_DIR}/python)
if (DEEPSEA_INSTALL)
	install(DIRECTORY python DESTINATION share/deepsea COMPONENT dev USE_SOURCE_PERMISSIONS
		PATTERN __pycache__ EXCLUDE)
endif()

add_subdirectory(modules)

if (DEEPSEA_BUILD_TESTS)
	add_subdirectory(testers)
endif()

# Documentation. (populated by built libraries)
add_subdirectory(doc)
