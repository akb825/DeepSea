ds_add_module_library(deepsea_geometry deepsea_math)
ds_install_library(TARGET deepsea_geometry MODULE Geometry DEPENDENCIES Math)

add_subdirectory(test)
