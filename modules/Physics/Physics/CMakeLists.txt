file(GLOB_RECURSE sources src/*.c src/*.cpp src/*.h include/*.h include/*.mslh)
ds_add_library(deepsea_physics MODULE Physics FILES ${sources})
ds_install_library(TARGET deepsea_physics MODULE Physics DEPENDS Geometry)
