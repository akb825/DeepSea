file(GLOB_RECURSE sources src/*.c src/*.h include/*.h include/*.mslh)
ds_add_library(deepsea_animation MODULE Animation FILES ${sources})
ds_install_library(TARGET deepsea_animation MODULE Animation DEPENDS Render)
