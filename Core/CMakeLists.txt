find_package(Threads)

file(GLOB_RECURSE sources src/*.c src/*.h include/*.h)
add_library(deepsea_core ${DEEPSEA_LIB} ${sources})

target_link_libraries(deepsea_core PUBLIC
	${CMAKE_THREAD_LIBS_INIT})

install_library(TARGET deepsea_core MODULE Core)
