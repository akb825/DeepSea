ds_glob_library_sources(sources Geometry src/*.c src/*.h include/*.h)
ds_add_library(deepsea_geometry Geometry ${sources})
