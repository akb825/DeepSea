# Application

DeepSea Application library contains utilities common across applications. This primarily consists of events, such as input and window resize events.

# Main function

Each application implementation must provide a special main static library. This is used to bootstrap main for platforms that have special requirements. For example, Windows requires a `WinMain()` for non-console applications, Android requires a function to be loaded by a native activity.

The main function must have the following signature with C linkage:

	int dsMain(int argc, const char** argv)

The executable that defines the `dsMain()` function must also link to the static main library that corresponds to the application implementation.

The module name for `find_package()` and library name are the standard name appended with "Main" or "main". For example, `ApplicationSDL` has the `ApplicationSDLMain` module name, with the target name `deesea_application_sdl_main`.
