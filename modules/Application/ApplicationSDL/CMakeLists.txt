find_package(SDL2 QUIET)
if (NOT SDL2_FOUND)
	message("SDL2 not found. Skipping ApplicationSDL.")
	return()
endif()

if (APPLE)
	set(extraPatterns src/*.m)
endif()
file(GLOB_RECURSE sources src/*.c src/*.h include/*.h ${extraPatterns})

ds_add_library(deepsea_application_sdl MODULE ApplicationSDL FILES ${sources}
	DEPENDS deepsea_application)
ds_target_include_directories(deepsea_application_sdl PRIVATE ${SDL2_INCLUDE_DIR})
ds_target_link_libraries(deepsea_application_sdl PRIVATE ${SDL2_LIBRARY})

# Special library to contain the main wrapper.
file(GLOB_RECURSE mainSources main/*.c)
add_library(deepsea_application_sdl_main STATIC ${mainSources})
ds_setup_filters(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/main FILES ${mainSources})
target_link_libraries(deepsea_application_sdl_main PUBLIC ${SDL2_LIBRARY})
if (NOT IOS)
	# Main defined in primary SDL library for iOS.
	target_link_libraries(deepsea_application_sdl_main PUBLIC ${SDL2MAIN_LIBRARY})
endif()
target_include_directories(deepsea_application_sdl_main
	PRIVATE ${SDL2_INCLUDE_DIR} ${DEEPSEA_MODULE_DIR}/Core/Core/include)
ds_set_folder(deepsea_application_sdl_main modules)

ds_install_library(TARGET deepsea_application_sdl_main MODULE ApplicationSDLMain)
ds_install_library(TARGET deepsea_application_sdl MODULE ApplicationSDL DEPENDS Application
	ApplicationSDLMain)

add_subdirectory(test)
