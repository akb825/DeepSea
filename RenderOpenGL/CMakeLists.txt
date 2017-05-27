find_package(OpenGL)
if (NOT OPENGL_FOUND)
	return()
endif()

ds_add_module_library(deepsea_render_opengl deepsea_render)
ds_install_library(TARGET deepsea_render_opengl MODULE RenderOpenGL DEPENDENCIES Render)
