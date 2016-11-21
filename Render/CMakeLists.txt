file(GLOB_RECURSE sources src/*.c src/*.h include/*.h)
add_library(deepsea_render ${DEEPSEA_LIB} ${sources})

target_link_libraries(deepsea_render PUBLIC
	deepsea_core
	deepsea_math)
target_compile_definitions(deepsea_math PUBLIC _USE_MATH_DEFINES)
	
set_folder(deepsea_render libs)
setup_filters(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src
	INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/DeepSea/Render
	FILES ${sources})

install_library(TARGET deepsea_render MODULE Render DEPENDENCIES Core Math)
set(DEEPSEA_DOC_PROJECTS ${DEEPSEA_DOC_PROJECTS} Render PARENT_SCOPE)
