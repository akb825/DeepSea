file(GLOB_RECURSE sources src/*.c src/*.h include/*.h)
add_library(deepsea_geometry ${DEEPSEA_LIB} ${sources})

target_link_libraries(deepsea_geometry PUBLIC
	deepsea_math)
	
ds_set_folder(deepsea_geometry libs)
ds_setup_filters(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src
	INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/DeepSea/Geometry
	FILES ${sources})

ds_install_library(TARGET deepsea_geometry MODULE Geometry DEPENDENCIES Math)
set(DEEPSEA_DOC_PROJECTS ${DEEPSEA_DOC_PROJECTS} Geometry PARENT_SCOPE)

add_subdirectory(test)
