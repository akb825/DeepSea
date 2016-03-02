# Introduction

DeepSea is a game engine written in C.

# Building

Building is done with CMake. The following options may be used:

* **-DCMAKE\_BUILD\_TYPE=Debug|Release**: Building in Debug or Release. This should always be specified.
* **-DDEEPSEA\_SHARED=ON**: Build with shared libraries. Default is to build with static libraries.

Once you have built and installed DeepSea, and have added the `lib/cmake/DeepSea` directory to `CMAKE_PREFIX_PATH`, you can find the various modules with the `find_package()` CMake function. For example:

    find_package(DeepSea MODULES Core Math Graphics)

Libraries and include directories can be found through the `DEEPSEA_MODULE_LIBRARIES` and `DEEPSEA_MODULE_NCLUDE_DIRS` CMake variables. For example: `DEEPSEA_CORE_LIBRARIES` and `DEEPSEA_CORE_INCLUDE_DIRS`.
