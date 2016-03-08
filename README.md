# Introduction

DeepSea is a game engine written in C.

# Dependencies

The following software is required to build DeepSea:

* cmake 3.0.2 or later
* doxygen (optional)
* gtest (optional)

# Platforms

DeepSea has been built for and tested on the following platforms:

* Linux
* Windows

# Building

Building is done with CMake. To build a release build, execute the following commands:

	DeepSea$ mkdir build
	DeepSea$ cd build
	DeepSea/build$ cmake .. -DCMAKE_BUILD_TYPE=Release
	DeepSea/build$ make

The following options may be used when running cmake:

* `-DCMAKE_BUILD_TYPE=Debug|Release`: Building in Debug or Release. This should always be specified.
* `-DDEEPSEA_SHARED=ON`: Build with shared libraries. Default is to build with static libraries.

Once you have built and installed DeepSea, and have added the `lib/cmake/DeepSea` directory to `CMAKE_PREFIX_PATH`, you can find the various modules with the `find_package()` CMake function. For example:

    find_package(DeepSea MODULES Core Math Graphics)

Libraries and include directories can be found through the `DeepSeaModule_LIBRARIES` and `DeepSeaModule_NCLUDE_DIRS` CMake variables. For example: `DeepSeaCore_LIBRARIES` and `DeepSeaCore_INCLUDE_DIRS`.

# Modules

DeepSea contains the following modules:

* [Core](Core/README.md): Core functionality including logging, debugging, memory managment, threading, and IO.

