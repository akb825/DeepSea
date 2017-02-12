ds_add_module_library(deepsea_math deepsea_core)
ds_install_library(TARGET deepsea_math MODULE Math DEPENDENCIES Core)

add_subdirectory(test)
