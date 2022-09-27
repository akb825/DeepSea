file(GLOB_RECURSE sources src/*.c src/*.h include/*.h)
ds_add_library(deepsea_application MODULE Application FILES ${sources})
ds_install_library(TARGET deepsea_application MODULE Application DEPENDS Render)
