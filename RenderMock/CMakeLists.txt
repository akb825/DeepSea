file(GLOB_RECURSE sources src/*.c src/*.h include/*.h)
add_library(deepsea_render_mock ${DEEPSEA_LIB} ${sources})

target_link_libraries(deepsea_render_mock PUBLIC
	deepsea_render)
	
ds_set_folder(deepsea_render_mock libs)
ds_setup_filters(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src
	INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/DeepSea/Render/Mock
	FILES ${sources})

ds_install_library(TARGET deepsea_render_mock MODULE RenderMock DEPENDENCIES Render)
set(DEEPSEA_DOC_PROJECTS ${DEEPSEA_DOC_PROJECTS} RenderMock PARENT_SCOPE)

add_subdirectory(test)
