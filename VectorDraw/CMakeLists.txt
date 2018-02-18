file(GLOB_RECURSE sources src/*.c src/*.h include/*.h include/*.mslh)
ds_add_library(deepsea_vector_draw MODULE VectorDraw FILES ${sources} DEPENDS deepsea_render
	deepsea_text)
ds_install_library(TARGET deepsea_vector_draw MODULE VectorDraw DEPENDS Render Text)

add_subdirectory(test)
