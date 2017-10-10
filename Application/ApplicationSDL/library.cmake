ds_glob_library_sources(sources Application/ApplicationSDL src/*.c src/*.h include/*.h)
ds_add_library(deepsea_application_sdl Application/ApplicationSDL ${sources})
ds_target_include_directories(deepsea_application_sdl PRIVATE ${SDL2_INCLUDE_DIR})
ds_target_link_libraries(deepsea_application_sdl PRIVATE ${SDL2_LIBRARIES})
