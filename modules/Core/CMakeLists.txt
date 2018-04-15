find_package(Threads)

file(GLOB_RECURSE sources src/*.c src/*.h include/*.h)
ds_add_library(deepsea_core MODULE Core FILES ${sources})

ds_target_link_libraries(deepsea_core PUBLIC ${CMAKE_THREAD_LIBS_INIT})

if (DEEPSEA_PROFILING)
	set(profilingEnabled 1)
else()
	set(profilingEnabled 0)
endif()
ds_target_compile_definitions(deepsea_core PUBLIC DS_PROFILING_ENABLED=${profilingEnabled})

ds_install_library(TARGET deepsea_core MODULE Core)

add_subdirectory(test)
