find_package(Threads)

file(GLOB_RECURSE sources src/*.c src/*.h include/*.h)
add_library(deepsea_core ${DEEPSEA_LIB} ${sources})

target_include_directories(deepsea_core PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>)

target_link_libraries(deepsea_core PUBLIC
	${CMAKE_THREAD_LIBS_INIT})

install_library(TARGET deepsea_core MODULE Core)
