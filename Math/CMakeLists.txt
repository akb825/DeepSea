find_package(Threads)

file(GLOB_RECURSE sources src/*.c src/*.h include/*.h)
add_library(deepsea_math ${DEEPSEA_LIB} ${sources})

find_library(MATH_LIB m)
if (${MATH_LIB} STREQUAL MATH_LIB-NOTFOUND)
	set(MATH_LIB "")
endif()

target_link_libraries(deepsea_math PUBLIC
	deepsea_core
	${MATH_LIB})
	
set_folder(deepsea_math libs)
setup_filters(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src
	INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/DeepSea/MATH
	FILES ${sources})

install_library(TARGET deepsea_math MODULE Math DEPENDENCIES Core)
set(DEEPSEA_DOC_PROJECTS ${DEEPSEA_DOC_PROJECTS} Math PARENT_SCOPE)

add_subdirectory(test)
