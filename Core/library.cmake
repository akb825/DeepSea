find_package(Threads)

ds_glob_library_sources(sources Core src/*.c src/*.h include/*.h)
ds_add_library(deepsea_core Core ${sources})

ds_target_link_libraries(deepsea_core PUBLIC ${CMAKE_THREAD_LIBS_INIT})

if (DEEPSEA_PROFILING)
	set(profilingEnabled 1)
else()
	set(profilingEnabled 0)
endif()
ds_target_compile_definitions(deepsea_core PUBLIC DS_PROFILING_ENABLED=${profilingEnabled})
