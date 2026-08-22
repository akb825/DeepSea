# Application

DeepSea Application library contains utilities common across applications. This primarily consists of events, such as input and window resize events.

# Main function

Each application implementation must provide a special main static library. This is used to bootstrap main for platforms that have special requirements. For example, Windows requires a `WinMain()` for non-console applications, Android requires a function to be loaded by a native activity. It is also responsible for driving the run loop of the application, as some targets work best with or require control over dispatching of events or triggering updates and draws.

The main function must have the following signature with C linkage:

	dsApplication* dsMain(int argc, const char* const* argv)

After returning, the application library will run the event and update loops until a quit is requested. If `NULL` is returned, the application will quit immediately with a non-zero error code. The executable that defines the `dsMain()` function must also link to the static main library that corresponds to the application implementation.

The module name for `find_package()` and library name are the standard name appended with "Main" or "main". For example, `ApplicationSDL` has the `ApplicationSDLMain` module name, with the target name `deesea_application_sdl_main`.
