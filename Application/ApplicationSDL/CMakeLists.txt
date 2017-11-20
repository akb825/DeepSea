find_package(SDL2 QUIET)
if (NOT SDL2_FOUND)
	message("SDL2 not found. Skipping ApplicationSDL")
	return()
endif()

ds_add_module_library(deepsea_application_sdl deepsea_application)
ds_install_library(TARGET deepsea_application_sdl MODULE ApplicationSDL DEPENDENCIES Application)

# Special library to contain the main wrapper.
file(GLOB_RECURSE mainSources main/*.c)
add_library(deepsea_application_sdl_main STATIC ${mainSources})
ds_setup_filters(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/main FILES ${mainSources})
target_link_libraries(deepsea_application_sdl_main PUBLIC ${SDL2MAIN_LIBRARY})
target_include_directories(deepsea_application_sdl_main PRIVATE ${SDL2_INCLUDE_DIR})
ds_install_library(TARGET deepsea_application_sdl_main MODULE ApplicationSDLMain
	EXTERNAL_DEPENDENCIES SDL2)

add_subdirectory(test)
