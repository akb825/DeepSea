file(GLOB_RECURSE sources src/*.c src/*.h include/*.h)
ds_add_library(deepsea_particle MODULE Particle FILES ${sources} DEPENDS deepsea_render)
ds_target_include_directories(deepsea_particle PRIVATE src)
ds_install_library(TARGET deepsea_particle MODULE Particle DEPENDS Render)

add_subdirectory(test)
