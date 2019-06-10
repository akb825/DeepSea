file(GLOB_RECURSE sources src/*.c src/*.h include/*.h include/*.mslh)
ds_add_library(deepsea_scene MODULE Scene FILES ${sources} DEPENDS deepsea_render)
ds_install_library(TARGET deepsea_scene MODULE Scene DEPENDS Render)
