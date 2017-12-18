file(GLOB_RECURSE sources src/*.c src/*.h include/*.h)
ds_add_library(deepsea_geometry MODULE Geometry FILES ${sources} DEPENDS deepsea_math)
ds_install_library(TARGET deepsea_geometry MODULE Geometry DEPENDS Math)

add_subdirectory(test)
