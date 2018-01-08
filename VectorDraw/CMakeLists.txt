file(GLOB_RECURSE sources src/*.c src/*.h include/*.h)
ds_add_library(deepsea_vector_draw MODULE VectorDraw FILES ${sources} DEPENDS deepsea_render
	deepsea_text)
ds_install_library(TARGET deepsea_vector_draw MODULE VectorDraw DEPENDS Render Text)
