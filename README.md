# Introduction

DeepSea is a game engine written in C. It is designed to be modular, allowing only pieces of the engine to be taken. For example, you can only take the Graphics module (and dependencies) without compiling the other modules.

# Dependencies

The following software is required to build DeepSea:

* cmake 3.0.2 or later
* doxygen (optional)
* gtest (optional)

# Platforms

DeepSea has been built for and tested on the following platforms:

* Linux (GCC and LLVM clang)
* Windows (requires Visual Studio 2015 or later)

# Building

Building is done with CMake. To build a release build, execute the following commands:

	DeepSea$ mkdir build
	DeepSea$ cd build
	DeepSea/build$ cmake .. -DCMAKE_BUILD_TYPE=Release
	DeepSea/build$ make

The following options may be used when running cmake:

## Compile Options:

* `-DCMAKE_BUILD_TYPE=Debug|Release`: Building in `Debug` or `Release`. This should always be specified.
* `-DCMAKE_INSTALL_PREFIX=path`: Sets the path to install to when running make install.
* `-DDEEPSEA_SHARED=ON|OFF`: Set to `ON` to build with shared libraries, `OFF` to build with static libraries. Default is `OFF`.
* `-DDEEPSEA_PROFILING=ON|OFF`: Set to `ON` to enable profiling of the code, `OFF` to compile out all profiling macros. Default is `ON`.

## Enabled Builds

* `-DDEEPSEA_BUILD_TESTS=ON|OFF`: Set to `ON` to build the unit tests. `gtest` must also be found in order to build the unit tests. Defaults to `ON`.
* `-DDEEPSEA_BUILD_DOCS=ON|OFF`: Set to `ON` to build the documentation. `doxygen` must also be found in order to build the documentation. Defaults to `ON`.
* `-DDEEPSEA_BUILD_RENDER=ON|OFF`: Set to `ON` to build the libraries related to rendering. Defaults to `ON`.

## Miscellaneous Options:

* `-DDEEPSEA_EXPORTS_DIR=directory`: The folder to place the cmake exports when building. This directory can be added to the module path when embedding in other projects to be able to use the `library_find()` cmake function. Defaults to `${CMAKE_BINARY_DIR}/cmake`.
* `-DDEEPSEA_ROOT_FOLDER=folder`: The root folder for the projects in IDEs that support them. (e.g. Visual Studio or XCode) This is useful if embedding DeepSea in another project. Defaults to DeepSea.

Once you have built and installed DeepSea, and have added the `lib/cmake/DeepSea` directory to `CMAKE_PREFIX_PATH`, you can find the various modules with the `find_package()` CMake function. For example:

    find_package(DeepSea MODULES Core Math Graphics)

Libraries and include directories can be found through the `DeepSeaModule_LIBRARIES` and `DeepSeaModule_INCLUDE_DIRS` CMake variables. For example: `DeepSeaCore_LIBRARIES` and `DeepSeaCore_INCLUDE_DIRS`.

# Modules

DeepSea contains the following modules:

* [Core](Core/README.md): (Required) Core functionality including logging, debugging, memory managment, threading, and Streams. See Core for general notes about the object and memory model used throughout all modules.
* [Math](Math/README.md): (Required) Math structures and functions used throughout DeepSea.
* [Geometry](Geometry/README.md): (Optional) Geometry classes typically used in graphics applications. This will be built with the graphics libraries.
* [Render](Render/README.md): (Optional) Interface to the rendering engine. This provides the interface that will be implemented for various system graphics APIs.

The directory structure of the include files is:

	DeepSea/<ModuleName>/[Subdirectory/]Header.h

For example:

	#include <DeepSea/Core/Config.h>
	#include <DeepSea/Core/Thread/Thread.h>
