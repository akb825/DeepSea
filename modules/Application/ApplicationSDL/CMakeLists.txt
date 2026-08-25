ds_find_package(SDL3 QUIET)
if (NOT SDL3_FOUND)
	message("SDL3 not found. Skipping ApplicationSDL.")
	return()
endif()

if (APPLE)
	set(extraPatterns src/*.m)
endif()
file(GLOB_RECURSE sources src/*.c src/*.h include/*.h ${extraPatterns})

ds_add_library(deepsea_application_sdl MODULE ApplicationSDL FILES ${sources})
# Prefer static linking if possible.
if (TARGET SDL3::SDL3-static)
	ds_target_link_libraries(deepsea_application_sdl PRIVATE SDL3::SDL3-static)
else()
	ds_target_link_libraries(deepsea_application_sdl PRIVATE SDL3::SDL3-shared)
endif()

# Special library to contain the main wrapper.
file(GLOB_RECURSE mainSources main/*.c)
add_library(deepsea_application_sdl_main STATIC ${mainSources})
ds_setup_filters(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/main FILES ${mainSources})
target_link_libraries(deepsea_application_sdl_main PRIVATE deepsea_application_sdl SDL3::Headers)
ds_set_folder(deepsea_application_sdl_main modules)

# Only need to list SDL3 as a dependency if static linking.
if (NOT DEEPSEA_SHARED)
	set(externalDeps "SDL3 REQUIRED")
else()
	set(externalDeps)
endif()

ds_install_library(TARGET deepsea_application_sdl_main MODULE ApplicationSDLMain
	DEPENDS ApplicationSDL EXTERNAL_DEPENDS ${externalDeps})
ds_install_library(TARGET deepsea_application_sdl MODULE ApplicationSDL DEPENDS Application)

add_subdirectory(test)
