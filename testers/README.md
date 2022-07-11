# GUI Testers

The GUI testers in this directory demonstrate various features of the engine. These provide a way to visually verify that the features work correctly in a way you can't with typical unit tests.

If parts of the engine are disabled through CMake variables, any of the the testers that depend on those features will be disabled as well. Each tester requires at a minimum a non-mock Render implementation and ApplicationSDL. Testers can run on all supported platforms: Windows, Linux, macOS, Android, and iOS.

## Common command-line options

The following command-line options are shared across all testers:

* `-r/--renderer <renderer>`: choose which render implementation to use. Possible values of `<renderer> are `Metal`, `Vulkan`, or `OpenGL`, case insensitive. The default is `Metal` for Apple platforms and `Vulkan` for non-apple platforms, falling back to `OpenGL` when `Vulkan` isn't supported. In practice, this option is primarily used to force `OpenGL`.
* `-d/--device <device>`: choose which device to use as a case-insensitive substring of the device name. For example, on a laptop that has both a NVidia and Intel GPU, you can pass `intel` or `nvidia` to choose between them. This is currently only implemented for the `Vulkan` renderer. By default discrete GPUs are chosen before integrated GPUs.

## Testers

The following testers are available:

* [Test Cube](TestCube/README.md)
* [Test Render Subpass](TestRenderSubpass/README.md)
* [Test Text](TestText/README.md)
