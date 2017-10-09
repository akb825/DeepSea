find_package(SDL2 QUIET)
if (NOT SDL2_FOUND)
	message("SDL2 not found. Skipping ApplicationSDL")
	return()
endif()

ds_add_module_library(deepsea_application_sdl deepsea_application)
ds_install_library(TARGET deepsea_application_sdl MODULE ApplicationSDL DEPENDENCIES Application)

add_subdirectory(test)
