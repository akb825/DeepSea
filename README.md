# Introduction

[![DeepSea](https://github.com/akb825/DeepSea/actions/workflows/main.yml/badge.svg)](https://github.com/akb825/DeepSea/actions/workflows/main.yml)

DeepSea is a game engine written in C. It is designed to be modular, allowing only pieces of the engine to be taken. For example, you can only take the Graphics module (and dependencies) without compiling the other modules.

# Dependencies

The following software is required to build DeepSea:

* [cmake](https://cmake.org/) 3.5 or later
* [Modular Shader Language](https://github.com/akb825/ModularShaderLanguage) (required for rendering, provided as submodule; will only build the client library without tests, which doesn't have extra required dependencies; tool should be downloaded via `update.sh` or built separately and available on `PATH` to compile shaders)
* [EasyProfiler](https://github.com/yse/easy_profiler) (optional default profiling implementation, provided as submodule)
* [SDL2](https://www.libsdl.org/) (optional)
* [FreeType](https://www.freetype.org/) (required for text)
* [HarfBuzz](https://github.com/harfbuzz/harfbuzz) (required for text)
* [SheenBidi](https://github.com/mta452/SheenBidi) (required for text, provided as submodule)
* [doxygen](https://doxygen.nl/) (optional)
* [gtest](https://github.com/google/googletest) (optional)
* [Cuttlefish](https://github.com/akb825/Cuttlefish) (recommended to create textures, required for scene and vector image conversion scripts)
* [VertexFormatConvert](https://github.com/akb825/VertexFormatConvert) (required for scene conversion scripts)
* [python](https://www.python.org/) 2.7 or 3.x (optional for flatbuffer converters)

The `update.sh` script may be used to update the code, submodules, and download pre-built binaries for the tools used for building and libraries. In the case of Windows, this script should be run in `git bash` (installed with Git for Windows) or similar environment. Apart from git, it will call into the following tools, which should be installed on most systems already:

* curl
* tar (for all but Windows packages)
* unzip (for Windows packages)

The first time you run `update.sh`, pass in the `-t` option to download the tools. You can also pass the `-l` with the list of platforms to download the libraries for. Supported platforms are:

* linux (Linux with glibc 2.31 for x86-64, e.g. Ubuntu 20.04)
* mac (macOS 10.11 for x86-64/arm64)
* mac-x86_64 (macOS 10.11 for x86-64)
* mac-arm64 (macOS 10.11 for arm64)
* win32 (Windows for x86, VS2017 runtime)
* win64 (Windows for x86-64, VS2017 runtime)
* android-x86 (Android for x86)
* android-x86_64 (Android for x86-64)
* android-armeabi-v7a (Android for ARM v7a)
* android-arm64-v8a (Android for ARM64 v8a)
* android-all (Convenience option to download all Android platforms)

After you have chosen the platforms to download libraries for, run `update.sh -a` at any point to pull the current branch and update the submodules, tools, and libraries to the proper versions. If you check out a specific revision or tag, you can run `update.sh -m` to update everything but the current git branch.

For example, if you want to build for both 32-bit and 64-bit Windows, your first call to update.sh would be:

	./update.sh -m -t -l win32 win64

This will download the submodules, tools, and pre-built libraries. After this point, you can run either `./update.sh -a` to update git and all dependencies or `./update.sh -m` to just update the dependencies.

> **Note:** When building on Linux, the freetype, harfbuzz, and SDL libraries aren't installed with the pre-built library packages since they are installed on nearly all Linux systems already. The development packages for these libraries must be installed when building DeepSea. In the case of Ubuntu, the `libfreetype6-dev`, `libharfbuzz-dev`, and `libsdl2-dev` should be installed. The `libgl1-mesa-dev` package is also required to compile the OpenGL backend.

> **Note:** When updating on Windows, possible running `update.sh -a` will fail if the `update.sh` script was updated due to file locking. If this happens, run `git pull` manually before calling into the update script.

# Platforms

DeepSea has been built for and tested on the following platforms:

* Linux (GCC and LLVM clang)
* Windows (requires Visual Studio 2015 or later)
* macOS

# Building

[CMake](https://cmake.org/) is used as the build system. The way to invoke CMake differs for different platforms.

## Linux

To create a release build, execute the following commands:

	DeepSea$ mkdir build
	DeepSea$ cd build
	DeepSea/build$ cmake .. -DCMAKE_BUILD_TYPE=Release
	DeepSea/build$ make

The tests can be run by running the command:

	DeepSea/build$ ctest

The executables can all be found under the `output` directory relative to your build directory.

## macOS

macOS can be built the same way as for Linux, or you can generate an Xcode project with the following commands:

	DeepSea$ mkdir build
	DeepSea$ cd build
	DeepSea/build$ cmake .. -G Xcode

## iOS

iOS can be built with an Xcode project similar to the macOS instructions above, pointing to the iOS toolchain.

	DeepSea$ mkdir build
	DeepSea$ cd build
	DeepSea/build$ cmake .. -G Xcode -DCMAKE_TOOLCHAIN_FILE=../cmake/ios.toolchain.cmake

## Windows

Generating Visual Studio projects can either be done through the CMake GUI tool or on the command line. To generate Visual Studio 2022 projects from the command line, you can run the commands:

	DeepSea$ mkdir build
	DeepSea$ cd build
	DeepSea\build$ cmake .. -G "Visual Studio 17 2022 Win64"

## Android

To build the examples, an Android Studio project is provided in the android subdirectory. Building the libraries can be accomplished through CMake with the CMake toolchain embedded in the Android NDK.

## Compile Options:

* `-DCMAKE_BUILD_TYPE=Debug|Release`: Building in `Debug` or `Release`. This should always be specified.
* `-DCMAKE_INSTALL_PREFIX=path`: Sets the path to install to when running make install.
* `-DDEEPSEA_SHARED=ON|OFF`: Set to `ON` to build with shared libraries, `OFF` to build with static libraries. Default is `OFF`.
* `-DDEEPSEA_SHARED=ON|OFF`: Set to `ON` to build all libraries into a single shared library. Default is `OFF`.
* `-DDEEPSEA_PROFILING=ON|OFF`: Set to `ON` to enable profiling of code, `OFF` to compile out all profiling macros. Default is `ON`.
* `-DDEEPSEA_GPU_PROFILING=ON|OFF`: Set to `ON` to enable profiling of the GPU, `OFF` to remove all GPU timing instrumentation. This can be used to independently disable GPU profiling while still leaving CPU profiling enabled. If `DEEPSEA_PROFILING` is set to `OFF`, then GPU profiling will also be disabled. Default is `OFF`.
* `-DDEEPSEA_SYSTEM_MSL=ON|OFF`: Set to `ON` to use the system installed version of Modular Shader Language, `OFF` to build the embedded submodule. Setting this to `ON` is useful when creating system packages, such as for a Linux distribution, but `OFF` is usually desired when cross-compiling for multiple platforms. When set to `ON`, you may need to have the lib/cmake/MSL directory (relative to the MSL install path) in `CMAKE_PREFIX_PATH`. Default is `OFF`.

## Enabled Builds

* `-DDEEPSEA_BUILD_TESTS=ON|OFF`: Set to `ON` to build the unit tests. `gtest` must also be found in order to build the unit tests. Defaults to `ON`.
* `-DDEEPSEA_BUILD_DOCS=ON|OFF`: Set to `ON` to build the documentation. `doxygen` must also be found in order to build the documentation. Defaults to `ON`.
* `-DDEEPSEA_BUILD_EASY_PROFILER=ON|OFF`: Set to `ON` to build the easy_profiler implementation for for profiling. Defaults to `ON`.
* `-DDEEPSEA_BUILD_RENDER=ON|OFF`: Set to `ON` to build the libraries related to rendering. Defaults to `ON`.
* `-DDEEPSEA_BUILD_RENDER_MOCK=ON|OFF`: Set to `ON` to build the mock render implementation, used for the renderer unit tests. Defaults to `ON`.
* `-DDEEPSEA_BUILD_RENDER_OPENGL=ON|OFF`: Set to `ON` to build the OpenGL render implementation. Defaults to `ON`.
* `-DDEEPSEA_BUILD_RENDER_VULKAN=ON|OFF`: Set to `ON` to build the Vulkan render implementation. Defaults to `ON`.
* `-DDEEPSEA_BUILD_RENDER_METAL=ON|OFF`: Set to `ON` to build the Metal render implementation. Defaults to `ON`.
* `-DDEEPSEA_BUILD_TEXT=ON|OFF`: Set to `ON` to build the text rendering library. Defaults to `ON`.
* `-DDEEPSEA_BUILD_VECTOR_DRAW=ON|OFF`: Set to `ON` to build the vector draw library. Defaults to `ON`.
* `-DDEEPSEA_BUILD_PARTICLE=ON|OFF`: Set to `ON` to build the particle library. Defaults to `ON`.
* `-DDEEPSEA_BUILD_ANIMATION=ON|OFF`: Set to `ON` to build the animation library. Defaults to `ON`.
* `-DDEEPSEA_BUILD_SCENE=ON|OFF`: Set to `ON` to build the scene library. Defaults to `ON`.
* `-DDEEPSEA_BUILD_PHYSICS=ON|OFF`: Set to `ON` to build the physics library. Defaults to `ON`.
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
* `-DDEEPSEA_INSTALL_SET_RPATH=ON|OFF`: Set rpath during install for the library and tool on installation. Set to `OFF` if including in another project that wants to control the rpath. Default is `ON`.
* `-DDEEPSEA_ANDROID_ASSETS_DIR=folder`: Folder relative to project app directory to place assets for Android. Defaults to `src/main/assets`.
* `-DDEEPSEA_NO_PREBUILT_LIBS=ON|OFF`: Don't use any pre-built library dependencies.
* `-DCMAKE_OSX_DEPLOYMENT_TARGET=version`: Minimum version of macOS to target when building for Mac. Defaults to 10.13, or 11.0 for ARM only, but may be set as low as 10.11.

Once you have built and installed DeepSea and set the base install directory to `CMAKE_PREFIX_PATH`, you can find the various modules with the `find_package()` CMake function. For example:

	find_package(DeepSea COMPONENTS Core Math Render)

You can either link to the `DeepSea::Module` target or use the `DeepSeaModule_LIBRARIES` and `DeepSeaModule_INCLUDE_DIRS` CMake variables, replacing `Module` with the module name. For example: `DeepSea::Core`, `DeepSeaCore_LIBRARIES`, and `DeepSeaCore_INCLUDE_DIRS`.

# Modules

DeepSea contains the following modules:

* [Core](modules/Core/Core/README.md): Core functionality including logging, debugging, memory managment, threading, and Streams. See Core for general notes about the object and memory model used throughout all modules.
* [Math](modules/Math/README.md): Math structures and functions used throughout DeepSea.
* [Geometry](modules/Geometry/README.md): Geometry classes typically used in graphics applications. This will be built with the graphics libraries.
* [Render](modules/Render/Render/README.md): Interface to the rendering engine. This provides the interface that will be implemented for various system graphics APIs.
* [RenderMock](modules/Render/RenderMock/README.md): Mock implementation of the Render library, used for unit tests.
* [RenderOpenGL](modules/Render/RenderOpenGL/README.md): OpenGL implementation of the Render library. This supports both desktop OpenGL and OpenGL ES.
* [RenderVulkan](modules/Render/RenderVulkan/README.md): Vulkan implementation of the Render library.
* [RenderMetal](modules/Render/RenderMetal/README.md): Metal implementation of the Render library.
* [RenderBootstrap](modules/Render/RenderBootstrap/README.md): Library that aids in creating one of the various renderers based on what is supported.
* [Text](modules/Text/README.md): Draws Unicode text.
* [VectorDraw](modules/VectorDraw/README.md): Draws vector graphics.
* [Particle](modules/Particle/README.md): Draws particles created by particle emitters.
* [Animation](modules/Animation/README.md): Handles animation of transform hierarchies and skinning.
* [Scene](modules/Scene/Scene/README.md): Scene library for creating scene graphs mixed with render passes and operations to perform each frame as a part of rendering.
* [SceneLighting](modules/Scene/SceneLighting/README.md): Library for managing lights and shadows within a scene.
* [SceneVectorDraw](modules/Scene/SceneVectorDraw/README.md): Library for drawing vector images and text within a scene.
* [SceneParticle](modules/Scene/SceneParticle/README.md): Library for managing particle emitters and particles within a scene.
* [Physics](modules/Physics/Physics/README.md): Interface to the physics engine. This provides the interface that can be implemented to integrate with 3rd party physics implementations.
* [Application](modules/Application/Application/README.md): Application library, providing functionality such as input and window events.
* [ApplicationSDL](modules/Application/ApplicationSDL/README.md): SDL implementation of the Application library.

The directory structure of the include files is:

	DeepSea/<ModuleName>/[Subdirectory/]Header.h

For example:

	#include <DeepSea/Core/Config.h>
	#include <DeepSea/Core/Thread/Thread.h>
