file(GLOB_RECURSE sources src/*.c src/*.h include/*.h include/*.mslh)
ds_add_library(deepsea_particle MODULE Particle FILES ${sources})
ds_target_include_directories(deepsea_particle PRIVATE src)
ds_install_library(TARGET deepsea_particle MODULE Particle DEPENDS Render)

add_subdirectory(test)
