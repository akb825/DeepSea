ds_start_modules()

# Required libraries.
ds_add_module(Core)
ds_add_module(Math)

# Render libraries
if (DEEPSEA_BUILD_RENDER)
	ds_add_module(Geometry)
	ds_add_module(Render)
	if (DEEPSEA_BUILD_RENDER_MOCK)
		ds_add_module(Render/RenderMock)
	endif()
	if (DEEPSEA_BUILD_RENDER_OPENGL)
		ds_add_module(Render/RenderOpenGL)
	endif()

	if (DEEPSEA_BUILD_TEXT)
		ds_add_module(Text)
		if (DEEPSEA_BUILD_VECTOR_DRAW)
			ds_add_module(VectorDraw)
		endif()
	endif()

	if (DEEPSEA_BUILD_APPLICATION)
		ds_add_module(Application)
		if (DEEPSEA_BUILD_APPLICATION_SDL)
			ds_add_module(Application/ApplicationSDL)
		endif()
	endif()
endif()

ds_finish_modules()
ds_install_master_config()
