find_package(Threads)

file(GLOB_RECURSE sources src/*.c src/*.h include/*.h)
add_library(deepsea_core ${DEEPSEA_LIB} ${sources})

target_link_libraries(deepsea_core PUBLIC
	${CMAKE_THREAD_LIBS_INIT})

if (DEEPSEA_PROFILING)
	set(profilingEnabled 1)
else()
	set(profilingEnabled 0)
endif()
target_compile_definitions(deepsea_core PUBLIC DS_PROFILING_ENABLED=${profilingEnabled})
	
set_folder(deepsea_core libs)
setup_filters(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src
	INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/DeepSea/Core
	FILES ${sources})

install_library(TARGET deepsea_core MODULE Core)
set(DEEPSEA_DOC_PROJECTS ${DEEPSEA_DOC_PROJECTS} Core PARENT_SCOPE)

add_subdirectory(test)
