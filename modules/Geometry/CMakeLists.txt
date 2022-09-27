file(GLOB_RECURSE sources src/*.c src/*.h src/*.cpp src/*.hpp include/*.h)
ds_add_library(deepsea_geometry MODULE Geometry FILES ${sources})
ds_install_library(TARGET deepsea_geometry MODULE Geometry DEPENDS Math)

add_subdirectory(test)
