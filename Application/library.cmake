ds_glob_library_sources(sources Application src/*.c src/*.h include/*.h)
ds_add_library(deepsea_application Application ${sources})
