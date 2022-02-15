ds_start_modules()

# Required libraries.
ds_add_module(Core/Core)
ds_add_module(Math)

# Profiling
if (DEEPSEA_PROFILING AND DEEPSEA_BUILD_EASY_PROFILER)
	ds_add_module(Core/EasyProfiler)
endif()

# Render libraries
if (DEEPSEA_BUILD_RENDER)
	ds_add_module(Geometry)
	ds_add_module(Render/Render)
	if (DEEPSEA_BUILD_RENDER_MOCK)
		ds_add_module(Render/RenderMock)
	endif()
	if (DEEPSEA_BUILD_RENDER_OPENGL)
		ds_add_module(Render/RenderOpenGL)
	endif()
	if (DEEPSEA_BUILD_RENDER_VULKAN)
		ds_add_module(Render/RenderVulkan)
	endif()
	if (DEEPSEA_BUILD_RENDER_METAL)
		ds_add_module(Render/RenderMetal)
	endif()
	ds_add_module(Render/RenderBootstrap)

	if (DEEPSEA_BUILD_TEXT)
		ds_add_module(Text)
		if (DEEPSEA_BUILD_VECTOR_DRAW AND TARGET deepsea_text)
			ds_add_module(VectorDraw)
		endif()
	endif()

	if (DEEPSEA_BUILD_PARTICLE)
		ds_add_module(Particle)
	endif()

	if (DEEPSEA_BUILD_SCENE)
		ds_add_module(Scene/Scene)
		ds_add_module(Scene/SceneLighting)
		if (TARGET deepsea_vector_draw)
			ds_add_module(Scene/VectorDrawScene)
		endif()
	endif()

	if (DEEPSEA_BUILD_APPLICATION)
		ds_add_module(Application/Application)
		if (DEEPSEA_BUILD_APPLICATION_SDL)
			ds_add_module(Application/ApplicationSDL)
		endif()
	endif()
endif()

ds_finish_modules()
ds_install_master_config()
