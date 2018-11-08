# Introduction

DeepSea is a game engine written in C. It is designed to be modular, allowing only pieces of the engine to be taken. For example, you can only take the Graphics module (and dependencies) without compiling the other modules.

# Dependencies

The following software is required to build DeepSea:

* [cmake](https://cmake.org/) 3.1 or later
* [Modular Shader Language](https://github.com/akb825/ModularShaderLanguage) (required for rendering, provided as submodule; will only build the client library without tests, which doesn't have extra required dependencies; tool should be built separately and available on `PATH` to compile shaders)
* [EasyProfiler](https://github.com/yse/easy_profiler) (optional default profiling implementation, provided as submodule)
* [SDL](https://www.libsdl.org/) 2.0.4 or later (optional)
* [FreeType](https://www.freetype.org/) (required for text)
* [HarfBuzz](https://www.freedesktop.org/wiki/Software/HarfBuzz/) (required for text)
* [SheenBidi](https://github.com/mta452/SheenBidi) (required for text, provided as submodule)
* [doxygen](http://www.stack.nl/~dimitri/doxygen/) (optional)
* [gtest](https://github.com/google/googletest) (optional)
* [Cuttlefish](https://github.com/akb825/Cuttlefish) (recommended to create textures)
* [python](https://www.python.org/) 2.7 or 3.x (optional for flatbuffer converters)

The `update.sh` script may be used to update the code, submodules, and download pre-built binaries for the tools used for building and libraries. In the case of Windows, this script should be run in `git bash` (installed with Git for Windows) or similar environment. Apart from git, it will call into the following tools, which should be installed on most systems already:

* curl
* tar (for all but Windows packages)
* unzip (for Windows packages)

The first time you run `update.sh`, pass in the `-t` option to download the tools. You can also pass the `-l` with the list of platforms to download the libraries for. Supported platforms are:

* linux (Linux with glibc 2.19 for x86-64)
* mac (macOS 10.11 for x86-64)
* win32 (Windows for x86)
* win64 (Windows for x86-64)
* android-x86 (Android for x86)
* android-x86_64 (Android for x86-64)
* android-armeabi-v7a (Android for ARM v7a)
* android-arm64-v8a (Android for ARM64 v8a)
* android-all (Convenience option to download all Android platforms)

After you have chosen the platforms to download libraries for, run `update.sh -a` at any point to pull the current branch and update the submodules, tools, and libraries to the proper versions. If you check out a specific revision or tag, you can run `update.sh -m` to update everything but the current git branch.

# Platforms

DeepSea has been built for and tested on the following platforms:

* Linux (GCC and LLVM clang)
* Windows (requires Visual Studio 2015 or later)
* Mac OS X

# Building

[CMake](https://cmake.org/) is used as the build system. The way to invoke CMake differs for different platforms.

## Linux/macOS

To create a release build, execute the following commands:

	DeepSea$ mkdir build
	DeepSea$ cd build
	DeepSea/build$ cmake .. -DCMAKE_BUILD_TYPE=Release
	DeepSea/build$ make

The tests can be run by running the command:

	DeepSea/build$ ctest

> **Note:** When building on Linux, the freetype and harfbuzz libraries aren't installed with the pre-built library packages since they are installed on nearly all Linux systems already. The development packages for these libraries must be installed when building DeepSea. In the case of Ubuntu, the `libfreetype6-dev` and `harfbuzz-dev` should be installed.

## Windows

Generating Visual Studio projects can either be done through the CMake GUI tool or on the command line. To generate Visual Studio 2017 projects from the command line, you can run the commands:

	DeepSea$ mkdir build
	DeepSea$ cd build
	DeepSea\build$ cmake .. -G "Visual Studio 15 2017 Win64"

## Compile Options:

* `-DCMAKE_BUILD_TYPE=Debug|Release`: Building in `Debug` or `Release`. This should always be specified.
* `-DCMAKE_INSTALL_PREFIX=path`: Sets the path to install to when running make install.
* `-DDEEPSEA_SHARED=ON|OFF`: Set to `ON` to build with shared libraries, `OFF` to build with static libraries. Default is `OFF`.
* `-DDEEPSEA_PROFILING=ON|OFF`: Set to `ON` to enable profiling of code, `OFF` to compile out all profiling macros. Default is `ON`.
* `-DDEEPSEA_GPU_PROFILING=ON|OFF`: Set to `ON` to enable profiling of the GPU, `OFF` to remove all GPU timing instrumentation. This can be used to independently disable GPU profiling while still leaving CPU profiling enabled. If `DEEPSEA_PROFILING` is set to `OFF`, then GPU profiling will also be disabled. Default is `ON`.
* `-DDEEPSEA_SYSTEM_MSL=ON|OFF`: Set to `ON` to use the system installed version of Modular Shader Language, `OFF` to build the embedded submodule. Setting this to `ON` is useful when creating system packages, such as for a Linux distribution, but `OFF` is usually desired when cross-compiling for multiple platforms. When set to `ON`, you may need to have the lib/cmake/MSL directory (relative to the MSL install path) in `CMAKE_PREFIX_PATH`. Default is `OFF`.

## Enabled Builds

* `-DDEEPSEA_BUILD_TESTS=ON|OFF`: Set to `ON` to build the unit tests. `gtest` must also be found in order to build the unit tests. Defaults to `ON`.
* `-DDEEPSEA_BUILD_DOCS=ON|OFF`: Set to `ON` to build the documentation. `doxygen` must also be found in order to build the documentation. Defaults to `ON`.
* `-DDEEPSEA_BUILD_EASY_PROFILER=ON|OFF`: Set to `ON` to build the easy_profiler implementation for for profiling. Defaults to `ON`.
* `-DDEEPSEA_BUILD_RENDER=ON|OFF`: Set to `ON` to build the libraries related to rendering. Defaults to `ON`.
* `-DDEEPSEA_BUILD_RENDER_MOCK=ON|OFF`: Set to `ON` to build the mock render implementation, used for the renderer unit tests. Defaults to `ON`.
* `-DDEEPSEA_BUILD_RENDER_OPENGL=ON|OFF`: Set to `ON` to build the OpenGL render implementation. Defaults to `ON`.
* `-DDEEPSEA_BUILD_RENDER_VULKAN=ON|OFF`: Set to `ON` to build the Vulkan render implementation. Defaults to `ON`.
* `-DDEEPSEA_BUILD_TEXT=ON|OFF`: Set to `ON` to build the text rendering library. Defaults to `ON`.
* `-DDEEPSEA_BUILD_VECTOR_DRAW=ON|OFF`: Set to `ON` to build the vector draw library. Defaults to `ON`.
* `-DDEEPSEA_BUILD_APPLICATION=ON|OFF`: Set to `ON` to build the application framework. Defaults to `ON`.
* `-DDEEPSEA_BUILD_APPLICATION_SDL=ON|OFF`: Set to `ON` to build the SDL application implementation. Defaults to `ON`.

## OpenGL specific Options

* `-DDEEPSEA_GLES=ON|OFF`: Set to `ON` to use OpenGL ES instead of desktop OpenGL. Defaults to `OFF`.
* `-DDEEPSEA_TARGET_GLES_VERSION=##`: Set to the target OpenGL ES version times 10. This is currently only used for Apple platforms, and will be the maximum version number supported. Defaults to `30`.
* `-DDEEPSEA_PREFER_EGL=ON|OFF`: Set to `ON` to use EGL instead of the platform-specifc loader when available. If EGL can't be found, it will fall back to the platform-specific loader. Defaults to `OFF`.

## Miscellaneous Options:

* `-DDEEPSEA_OUTPUT_DIR=directory`: The folder to place the output files. This may be explicitly left empty in order to output to the defaults used by cmake, but this may prevent tests and executables from running properly when `DEEPSEA_SHARED` is set to `ON`. Defaults to `${CMAKE_BINARY_DIR}/output`.
* `-DDEEPSEA_EXPORTS_DIR=directory`: The folder to place the cmake exports when building. This directory can be added to the module path when embedding in other projects to be able to use the `library_find()` cmake function. Defaults to `${CMAKE_BINARY_DIR}/cmake`.
* `-DDEEPSEA_ROOT_FOLDER=folder`: The root folder for the projects in IDEs that support them. (e.g. Visual Studio or XCode) This is useful if embedding DeepSea in another project. Defaults to DeepSea.
* `-DDEEPSEA_INSTALL=ON|OFF`: Allow installation for DeepSea components. This can be useful when embedding in other projects to prevent installations from including DeepSea. For example, when statically linking into a shared library. Defaults to `ON`.
* `-DDEEPSEA_ANDROID_ASSETS_DIR=folder`: Folder relative to project app directory to place assets for Android. Defaults to `src/main/assets`.
* `-DDEEPSEA_NO_PREBUILT_LIBS=ON|OFF`: Don't use any pre-built library dependencies.

Once you have built and installed DeepSea, you can find the various modules with the `find_package()` CMake function. For example:

	find_package(DeepSea CONFIG COMPONENTS Core Math Render)

Libraries and include directories can be found through the `DeepSeaModule_LIBRARIES` and `DeepSeaModule_INCLUDE_DIRS` CMake variables. For example: `DeepSeaCore_LIBRARIES` and `DeepSeaCore_INCLUDE_DIRS`.

> **Note:** In order for `find_package()` to succeed, on Windows you will need to add the path to `INSTALL_DIR/lib/cmake` to `CMAKE_PREFIX_PATH`. (e.g. `C:/Program Files/DeepSea/lib/cmake`) On other systems, if you don't install to a standard location, you will need to add the base installation path to `CMAKE_PREFIX_PATH`.

# Modules

DeepSea contains the following modules:

* [Core](modules/Core/README.md): Core functionality including logging, debugging, memory managment, threading, and Streams. See Core for general notes about the object and memory model used throughout all modules.
* [Math](modules/Math/README.md): Math structures and functions used throughout DeepSea.
* [Geometry](modules/Geometry/README.md): (Optional) Geometry classes typically used in graphics applications. This will be built with the graphics libraries.
* [Render](modules/Render/README.md): Interface to the rendering engine. This provides the interface that will be implemented for various system graphics APIs.
* [RenderMock](modules/Render/RenderMock/README.md): Mock implementation of the Render library, used for unit tests.
* [RenderOpenGL](modules/Render/RenderOpenGL/README.md): OpenGL implementation of the Render library. This supports both desktop OpenGL and OpenGL ES.
* [RenderVulkan](modules/Render/RenderVulkan/README.md): Vulkan implementation of the Render library.
* [RenderBootstrap](modules/Render/RenderVulkan/README.md): Library that aids in creating one of the various renderers based on what is supported.
* [Text](modules/Text/README.md): Draws Unicode text.
* [VectorDraw](modules/VectorDraw/README.md): Draws vector graphics.
* [Application](modules/Application/README.md): Application library, providing functionality such as input and window events.
* [ApplicationSDL](modules/Application/ApplicationSDL/README.md): SDL implementation of the Application library.

The directory structure of the include files is:

	DeepSea/<ModuleName>/[Subdirectory/]Header.h

For example:

	#include <DeepSea/Core/Config.h>
	#include <DeepSea/Core/Thread/Thread.h>
