find_package(SDL2 QUIET)
if (NOT SDL2_FOUND)
	message("SDL2 not found. Skipping ApplicationSDL.")
	return()
endif()

if (APPLE)
	set(extraPatterns src/*.m)
endif()
file(GLOB_RECURSE sources src/*.c src/*.h include/*.h ${extraPatterns})

ds_add_library(deepsea_application_sdl MODULE ApplicationSDL FILES ${sources})
ds_target_include_directories(deepsea_application_sdl PRIVATE ${SDL2_INCLUDE_DIRS})
ds_target_link_libraries(deepsea_application_sdl PRIVATE ${SDL2_LIBRARIES})

if (APPLE)
	if (IOS)
		ds_target_link_libraries(deepsea_application_sdl PRIVATE "-framework UIKit")
	else()
		ds_target_link_libraries(deepsea_application_sdl PRIVATE "-framework AppKit")
	endif()
endif()

# Special library to contain the main wrapper.
file(GLOB_RECURSE mainSources main/*.c)
add_library(deepsea_application_sdl_main STATIC ${mainSources})
ds_setup_filters(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/main FILES ${mainSources})
target_link_libraries(deepsea_application_sdl_main PRIVATE ${SDL2_LIBRARIES} ${SDL2MAIN_LIBRARIES})
target_include_directories(deepsea_application_sdl_main
	PRIVATE ${SDL2_INCLUDE_DIRS} ${DEEPSEA_MODULE_DIR}/Core/Core/include)
ds_set_folder(deepsea_application_sdl_main modules)

ds_install_library(TARGET deepsea_application_sdl_main MODULE ApplicationSDLMain)
ds_install_library(TARGET deepsea_application_sdl MODULE ApplicationSDL DEPENDS Application
	DEPENDS_NOLINK ApplicationSDLMain)

add_subdirectory(test)
