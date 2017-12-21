# Introduction

DeepSea is a game engine written in C. It is designed to be modular, allowing only pieces of the engine to be taken. For example, you can only take the Graphics module (and dependencies) without compiling the other modules.

# Dependencies

The following software is required to build DeepSea:

* [cmake](https://cmake.org/) 3.0.2 or later
* Modular Shader Language (required for rendering, provided as submodule; will only build the client library without tests, which doesn't have extra required dependencies; tool should be built separately to compile shaders)
* [SDL](https://www.libsdl.org/) 2.0.4 or later (optional)
* [FreeType](https://www.freetype.org/) (required for text)
* [HarfBuzz](https://www.freedesktop.org/wiki/Software/HarfBuzz/) (required for text)
* [doxygen](http://www.stack.nl/~dimitri/doxygen/) (optional)
* [gtest](https://github.com/google/googletest) (optional)
* [Cuttlefish](https://github.com/akb825/Cuttlefish) (recommended to create textures)

The submodule dependencies can be grabbed by running the commands

	DeepSea$ git submodule init
	DeepSea$ git submodule update

# Platforms

DeepSea has been built for and tested on the following platforms:

* Linux (GCC and LLVM clang)
* Windows (requires Visual Studio 2015 or later)
* Mac OS X

# Building

[CMake](https://cmake.org/) is used as the build system. The way to invoke CMake differs for different platforms.

## Dependencies

In order to perform a full build of DeepSea, the dependencies should be installed first.

* Modular Shader Language: the tool is required to build shaders. Download the repository and follow the instructions to build and install the tool. Make sure the tool is on `PATH`.
* [Cuttlefish](https://github.com/akb825/Cuttlefish): this is recommended to create textures. Download the repository and follow the instructions to build and install the tool. Make sure the tool is on `PATH`.
* [gtest](https://github.com/google/googletest): after installing the library you may need to set the `GTEST_ROOT` CMake variable to the installation location.
* [SDL](https://www.libsdl.org/): after installing the library you may need to set the `SDL_PATH` CMake variable to the installation location.
* [FreeType](https://www.freetype.org/): after installing the library, you may need to set the `FREETYPE_DIR` environment variable to the installation location.
* [HarfBuzz](https://www.freedesktop.org/wiki/Software/HarfBuzz/): after installing the library you may need to set the `HARFBUZZ_PATH` CMake variable to the installation location. Some notes when building HarfBuzz from source:
	* The [ragel](http://www.colm.net/open-source/ragel/) tool is required. Make sure that the `RAGEL` CMake variable is set to the path to `ragel`.
	* When building under Windows, building `ragel` must be done using [MinGW](https://sourceforge.net/projects/mingw/). Be sure to install the developer-tools, base, and gcc-g++ packages, then run `C:\MinGW\msys\1.0\msys.bat` for the console to perform the build. Also add `C:\MinGW\bin` to `PATH` to make sure `ragel` can be run oustide of a MinGW console.
	* The `HB_HAVE_FREETYPE` CMake variable should be set to `ON`. 
	* If you intend to build DeepSea as a shared library, it is recommended you build HarfBuzz as a shared library as well. This prevents you from manually linking HarfBuzz's direct dependencies with DeepSea as well.

## Linux/Mac OS X

To create a release build, execute the following commands:

	DeepSea$ mkdir build
	DeepSea$ cd build
	DeepSea/build$ cmake .. -DCMAKE_BUILD_TYPE=Release
	DeepSea/build$ make

The tests can be run by running the command:

	DeepSea/build$ ctest

## Windows

Building is generally performed through Visual Studio. This can either be done through the CMake GUI tool or on the command line. To generate Visual Studio 2017 projects from the command line, you can run the commands:

	DeepSea$ mkdir build
	DeepSea$ cd build
	DeepSea\build$ cmake .. -G "Visual Studio 15 2017 Win64"

## Compile Options:

* `-DCMAKE_BUILD_TYPE=Debug|Release`: Building in `Debug` or `Release`. This should always be specified.
* `-DCMAKE_INSTALL_PREFIX=path`: Sets the path to install to when running make install.
* `-DDEEPSEA_SHARED=ON|OFF`: Set to `ON` to build with shared libraries, `OFF` to build with static libraries. Default is `OFF`.
* `-DDEEPSEA_PROFILING=ON|OFF`: Set to `ON` to enable profiling of the code, `OFF` to compile out all profiling macros. Default is `ON`.
* `-DDEEPSEA_SYSTEM_MSL=ON|OFF`: Set to `ON` to use the system installed version of Modular Shader Language, `OFF` to build the embedded submodule. Setting this to `ON` is useful when creating system packages, such as for a Linux distribution, but `OFF` is usually desired when cross-compiling for multiple platforms. When set to `ON`, you may need to have the lib/cmake/MSL directory (relative to the MSL install path) in `CMAKE_PREFIX_PATH`. Default is `OFF`.

## Enabled Builds

* `-DDEEPSEA_BUILD_TESTS=ON|OFF`: Set to `ON` to build the unit tests. `gtest` must also be found in order to build the unit tests. Defaults to `ON`.
* `-DDEEPSEA_BUILD_DOCS=ON|OFF`: Set to `ON` to build the documentation. `doxygen` must also be found in order to build the documentation. Defaults to `ON`.
* `-DDEEPSEA_BUILD_RENDER=ON|OFF`: Set to `ON` to build the libraries related to rendering. Defaults to `ON`.
* `-DDEEPSEA_BUILD_RENDER_MOCK=ON|OFF`: Set to `ON` to build the mock render implementation, used for the renderer unit tests. Defaults to `ON`.
* `-DDEEPSEA_BUILD_RENDER_OPENGL=ON|OFF`: Set to `ON` to build the OpenGL render implementation. Defaults to `ON`.
* `-DDEEPSEA_BUILD_TEXT=ON|OFF`: Set to `ON` to build the text rendering library. Defaults to `ON`.
* `-DDEEPSEA_BUILD_APPLICATION=ON|OFF`: Set to `ON` to build the application framework. Defaults to `ON`.
* `-DDEEPSEA_BUILD_APPLICATION_SDL=ON|OFF`: Set to `ON` to build the SDL application implementation. Defaults to `ON`.

## OpenGL specific Options

* `-DDEEPSEA_GLES=ON|OFF`: Set to `ONE` to use OpenGL ES instead of desktop OpenGL. Defaults to `OFF`.
* `-DDEEPSEA_TARGET_GLES_VERSION=##`: Set to the target OpenGL ES version times 10. This is currently only used for Apple platforms, and will be the maximum version number supported. Defaults to `30`.

## Miscellaneous Options:

* `-DDEEPSEA_OUTPUT_DIR=directory`: The folder to place the output files. This may be explicitly left empty in order to output to the defaults used by cmake, but this may prevent tests and executables from running properly when `DEEPSEA_SHARED` is set to `ON`. Defaults to `${CMAKE_BINARY_DIR}/output`.
* `-DDEEPSEA_EXPORTS_DIR=directory`: The folder to place the cmake exports when building. This directory can be added to the module path when embedding in other projects to be able to use the `library_find()` cmake function. Defaults to `${CMAKE_BINARY_DIR}/cmake`.
* `-DDEEPSEA_ROOT_FOLDER=folder`: The root folder for the projects in IDEs that support them. (e.g. Visual Studio or XCode) This is useful if embedding DeepSea in another project. Defaults to DeepSea.
* `-DDEEPSEA_INSTALL=ON|OFF`: Allow installation for DeepSea components. This can be useful when embedding in other projects to prevent installations from including DeepSea. For example, when statically linking into a shared library. Defaults to `ON`.
* `-DDEEPSEA_FORCE_OSS_TEXT=ON|OFF`: Force the usage of OSS text libraries. (FreeType and HarfBuzz) Otherwise, platform-specific libraries will be used for Apple and Windows platforms. Defaults to `OFF`.

Once you have built and installed DeepSea, and have added the `lib/cmake/DeepSea` directory to `CMAKE_PREFIX_PATH`, you can find the various modules with the `find_package()` CMake function. For example:

	find_package(DeepSea COMPONENTS Core Math Render)

Libraries and include directories can be found through the `DeepSeaModule_LIBRARIES` and `DeepSeaModule_INCLUDE_DIRS` CMake variables. For example: `DeepSeaCore_LIBRARIES` and `DeepSeaCore_INCLUDE_DIRS`.

# Modules

DeepSea contains the following modules:

* [Core](Core/README.md): (Required) Core functionality including logging, debugging, memory managment, threading, and Streams. See Core for general notes about the object and memory model used throughout all modules.
* [Math](Math/README.md): (Required) Math structures and functions used throughout DeepSea.
* [Geometry](Geometry/README.md): (Optional) Geometry classes typically used in graphics applications. This will be built with the graphics libraries.
* [Render](Render/README.md): (Optional) Interface to the rendering engine. This provides the interface that will be implemented for various system graphics APIs.
* [RenderMock](Render/RenderMock/README.md): (Optional) Mock implementation of the Render library, used for unit tests.
* [RenderOpenGL](Render/RenderOpenGL/README.md): (Optional) OpenGL implementation of the Render library. This supports both desktop OpenGL and OpenGL ES.
* [Application](Application/README.md): (Optional) Application library, providing functionality such as input and window events.
* [ApplicationSDL](Application/ApplicationSDL/README.md): (Optional) SDL implementation of the Application library..

The directory structure of the include files is:

	DeepSea/<ModuleName>/[Subdirectory/]Header.h

For example:

	#include <DeepSea/Core/Config.h>
	#include <DeepSea/Core/Thread/Thread.h>
